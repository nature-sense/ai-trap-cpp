#include "ai_camera.h"
#include <filesystem>
#include <linux/videodev2.h>
namespace fs = std::filesystem;

const unsigned int ROI_CTRL_ID = 0x00982900;
const unsigned int NETWORK_FW_CTRL_ID = 0x00982901;

namespace io::naturesense {
    asio::awaitable<void> AiCamera::actor(
       Channel<CaptureReference> *tracking_in_chan,
       Channel<uint64_t> *recycle_chan,
       cv::Size hi_res_size,
       cv::Size lo_res_size,
       const char* network_file
       ){

        std::cout << "Starting AiCamera actor..." << std::endl;

        auto camera = AiCamera(tracking_in_chan, recycle_chan);
        camera.init(hi_res_size, lo_res_size, network_file);
        co_await camera.start();
    }

    AiCamera::AiCamera(Channel<CaptureReference> *tracking_in_chan, Channel<uint64_t> *recycle_chan) {
        this->tracking_in_chan = tracking_in_chan;
        this->recycle_chan = recycle_chan;
        this->device_fd = -1;

        cm = std::make_unique<CameraManager>();
        cm->start();
    }

    AiCamera::~AiCamera() {

    }

    int AiCamera::init(cv::Size hi_res_size, cv::Size lo_res_size, const char* network_file) {

        // ---------------------------------------------------------------
        // Find the IM500 camera
        // ---------------------------------------------------------------
        for (unsigned int i = 0; i < 16; i++)
        {
            const fs::path test_dir { "/sys/class/video4linux/v4l-subdev" + std::to_string(i) + "/device" };
            const fs::path module_dir { test_dir.string() + "/driver/module" };
            const fs::path id_dir { test_dir.string() + "/of_node" };

            if (fs::exists(module_dir) && fs::is_symlink(module_dir))
            {
                fs::path ln = fs::read_symlink(module_dir);
                if (ln.string().find("imx500") != std::string::npos)
                {
                    const std::string dev_node { "/dev/v4l-subdev" + std::to_string(i) };
                    device_fd = open(dev_node.c_str(), O_RDONLY, 0);

                    /* Find the progress indicator sysfs dev nodes. */
                    const std::string test_dir_str = fs::read_symlink(test_dir).string();
                    const std::size_t pos = test_dir_str.find_last_of("/") + 1;
                    assert(pos != std::string::npos);

                    const std::string imx500_device_id = test_dir_str.substr(pos);
                    std::string spi_device_id = imx500_device_id;
                    const std::size_t rep = spi_device_id.find("001a");
                    spi_device_id.replace(rep, 4, "0040");

                    break;
                }
            }
        }

        if (device_fd < 0)
            throw std::runtime_error("Cannot open imx500 device node");

        // ---------------------------------------------------------------
        // Load the network firmware
        // ---------------------------------------------------------------
        if (!fs::exists(network_file))
            throw std::runtime_error(network_file + " not found!");
        int fd = open(network_file, O_RDONLY, 0);

        v4l2_control ctrl { NETWORK_FW_CTRL_ID, fd };
        int ret = ioctl(device_fd, VIDIOC_S_CTRL, &ctrl);
        if (ret)
            throw std::runtime_error("failed to set network fw ioctl");
        close(fd);

        // ---------------------------------------------------------------
        // Set the ROI to be the hi_res size
        // ---------------------------------------------------------------
        Size s = full_sensor_resolution_.size().boundedToAspectRatio(Size(hi_res_size.width, hi_res_size.height));
        Rectangle roi = s.centeredTo(full_sensor_resolution_.center()).enclosedIn(full_sensor_resolution_);
        const uint32_t roi_array[4] = { (uint32_t)roi.x, (uint32_t)roi.y, (uint32_t)roi.width, (uint32_t)roi.height };

        v4l2_ext_control roi_ctrl;
        roi_ctrl.id = ROI_CTRL_ID;
        roi_ctrl.p_u32 = (unsigned int *)&roi_array;
        roi_ctrl.size = 16;

        v4l2_ext_controls ctrls;
        ctrls.count = 1;
        ctrls.controls = (v4l2_ext_control *)&roi_ctrl;

        ret = ioctl(device_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
        if (ret)
            throw std::runtime_error("IMX500: Unable to set absolute ROI");

        auto cameras = cm->cameras();
        if (cameras.empty()) {
            std::cout << "No cameras were identified on the system." << std::endl;
            cm->stop();
            return ENODEV;
        }

        std::string cameraId = cameras[0]->id();
        camera = cm->get(cameraId);
        camera->acquire();

        std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::VideoRecording, StreamRole::Viewfinder } );

        StreamConfiguration &hi_res_config = config->at(0);
        hi_res_config.size.width = hi_res_size.width;
        hi_res_config.size.height = hi_res_size.height;
        hi_res_config.pixelFormat = PixelFormat(formats::RGB888);

        StreamConfiguration &lo_res_config = config->at(1);
        lo_res_config.size.width = lo_res_size.width;
        lo_res_config.size.height = lo_res_size.height;
        lo_res_config.pixelFormat = PixelFormat(formats::RGB888);

        CameraConfiguration::Status validation = config->validate();
        if (validation == CameraConfiguration::Invalid) {
            throw std::runtime_error("failed to valid stream configurations");
        } else if (validation == CameraConfiguration::Adjusted) {
            std::cout << "Stream configuration adjusted " << std::endl;
        } else {
            std::cout << "Stream configuration valid " << std::endl;
        }

        camera->configure(config.get());

        std::cout << "Hi res configuration is: " << hi_res_config.toString() << std::endl;
        std::cout << "lo res configuration is: " << lo_res_config.toString() << std::endl;

        allocator = std::make_unique<FrameBufferAllocator>(camera);

        size_t n_buffers = INT_MAX;
        for (StreamConfiguration &cfg : *config) {
            ret = allocator->allocate(cfg.stream());
            if (ret < 0) {
                std::cerr << "Can't allocate buffers" << std::endl;
                return -ENOMEM;
            }

            size_t allocated = allocator->buffers(cfg.stream()).size();
            if (allocated < n_buffers) {
                n_buffers = allocated;
            }
        }
        std::cout << "Allocated " << n_buffers << " buffers for stream" << std::endl;

        hi_res_stream = std::make_unique<Stream*>(hi_res_config.stream());
        const std::vector<std::unique_ptr<FrameBuffer>> &hi_res_buffers = allocator->buffers(*hi_res_stream);

        lo_res_stream = std::make_unique<Stream*>(lo_res_config.stream());
        const std::vector<std::unique_ptr<FrameBuffer>> &lo_res_buffers = allocator->buffers(*lo_res_stream);

        auto cookie = std::bit_cast<u_int64_t>(this);

        for (unsigned int i = 0; i < n_buffers; ++i) {
            std::unique_ptr<Request> request = camera->createRequest(cookie);
            if (!request) {
                std::cerr << "Can't create request" << std::endl;
                return -ENOMEM;
            }

            auto req_adr = std::bit_cast<u_int64_t>(request.get());
            std::cerr << "Request " << req_adr << std::endl;

            const std::unique_ptr<FrameBuffer> &hi_res_buffer = hi_res_buffers[i];
            ret = request->addBuffer(*hi_res_stream, hi_res_buffer.get(), nullptr);
            if (ret < 0 ) {
                std::cerr << "Can't set hi res buffer for request"
                      << std::endl;
                return ret;
            }

            const std::unique_ptr<FrameBuffer> &lo_res_buffer = lo_res_buffers[i];
            ret = request->addBuffer(*lo_res_stream, lo_res_buffer.get(), nullptr);
            if (ret < 0) {
                std::cerr << "Can't set lo res buffer for request"
                      << std::endl;
                return ret;
            }
            requests[req_adr] = std::move(request);
        }

        camera->requestCompleted.connect(AiCamera::request_completed);
        std::cout << "Init camera - complete" << std::endl;

        return 0;
    }


    // The callback function provided to the camera
    void AiCamera::request_completed(Request *request) {

        // cast the request pointer to a u_int64_t to use as the
        // identity of the request
        u_int64_t req_adr = std::bit_cast<u_int64_t>(request);
        std::cout << "Request complete " << req_adr << std::endl;

        if (request->status() != Request::RequestCancelled) {
            const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

            // The cookie contains a reference to the AiCamera object
            uint64_t cookie = request->cookie();
            auto camera = std::bit_cast<AiCamera*>(cookie);

            // Get an image reference for the hi-res image
            const FrameBuffer* hi_res_buffer = buffers.at(*camera->hi_res_stream);
            auto hi_res_img_ref = ImageReference(hi_res_buffer->planes()[0]);

            // Get an image reference for the lo-res image
            const FrameBuffer* lo_res_buffer = buffers.at(*camera->lo_res_stream);
            auto lo_res_img_ref = ImageReference(lo_res_buffer->planes()[0]);

            // Get the output tensor from the metadata
            auto output = request->metadata().get(controls::rpi::CnnOutputTensor);
            auto info = request->metadata().get(controls::rpi::CnnOutputTensorInfo);


            // create the capture ref
            auto capture_ref = CaptureReference(
                req_adr,
                0,
                hi_res_img_ref,
                lo_res_img_ref );

            // Use a synchronous send because asynchronous functionality is mot
            auto res = camera->tracking_in_chan->try_send(boost::system::error_code{}, capture_ref);
            if (res == false) {
                std::cout << "Failed to send the CaptureReference " << std::endl;
            }
        }
    }
}