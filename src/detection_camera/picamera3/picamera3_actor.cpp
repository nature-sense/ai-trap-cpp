#include "picamera3_actor.h"
#include "actor_framework/mailbox.h"

namespace asio = boost::asio;
namespace experimental = asio::experimental;

namespace io::naturesense {
    Picamera3Actor::~Picamera3Actor() {
        Actor::~Actor();
        RecycleReferenceMixin::~RecycleReferenceMixin();
    };

    asio::awaitable<void> Picamera3Actor::post_async(RecycleReference ref) {
        co_await mailbox.async_send(error_code(), ref);
    }
    void Picamera3Actor::post_sync(RecycleReference ref) {
        mailbox.try_send(error_code(), ref);
    }

    void Picamera3Actor::link_actor(CaptureReferenceMixin* actor) {
        this->capture_ref_actor = actor;
    }

    void Picamera3Actor::initialise() {

        std::cout << "Initialising Picamera3Actor" << std::endl;

        int ret;

        // -----------------------------------------------------
        // Read the frame sizes from the system.toml file
        // -----------------------------------------------------
        auto conf = this->cfg[name];
        int hi_res_width  = conf["hi_res_size"]["width"].value_or(0);
        int hi_res_height  = conf["hi_res_size"]["height"].value_or(0);
        this->hi_res_size = cv::Size(hi_res_width, hi_res_height);

        int lo_res_width  = conf["lo_res_size"]["width"].value_or(0);
        int lo_res_height  = conf["lo_res_size"]["height"].value_or(0);
        this->lo_res_size = cv::Size(lo_res_width, lo_res_height);

        // -----------------------------------------------------
        // Create and start the camera manager
        // -----------------------------------------------------
        cm = std::make_unique<CameraManager>();
        cm->start();

        // -----------------------------------------------------
        // Get the list of cameras. Abort if there is no camera
        // -----------------------------------------------------
        auto cameras = cm->cameras();
        if (cameras.empty()) {
            cm->stop();
            std::cerr << "No cameras were identified on the system." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // -----------------------------------------------------
        // Chose the first camera from the list and acquire it
        // -----------------------------------------------------
        std::string cameraId = cameras[0]->id();
        camera = cm->get(cameraId);
        camera->acquire();

        // -----------------------------------------------------
        // Creat two streams using the default roles :
        // Video-recording Role
        // View-finder role
        // -----------------------------------------------------
        std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::VideoRecording, StreamRole::Viewfinder } );

        // -----------------------------------------------------
        // Modify the video-recording stream to become
        // the hi-res-stream
        // -----------------------------------------------------
        StreamConfiguration &hi_res_config = config->at(0);
        hi_res_config.size.width = hi_res_size.width;
        hi_res_config.size.height = hi_res_size.height;
        hi_res_config.pixelFormat = PixelFormat(formats::RGB888);

        // -----------------------------------------------------
        // Modify the viewfinder stream to become
        // the lo-res-stream
        // -----------------------------------------------------
        StreamConfiguration &lo_res_config = config->at(1);
        lo_res_config.size.width = lo_res_size.width;
        lo_res_config.size.height = lo_res_size.height;
        lo_res_config.pixelFormat = PixelFormat(formats::RGB888);


        // -----------------------------------------------------
        // Validate the configuration and abort if it is not valid
        // -----------------------------------------------------
        CameraConfiguration::Status validation = config->validate();
        if (validation == CameraConfiguration::Invalid) {
            std::abort();
        } else if (validation == CameraConfiguration::Adjusted) {
            std::cout << "Stream configuration adjusted " << std::endl;
        } else {
            std::cout << "Stream configuration valid " << std::endl;
        }

        // -----------------------------------------------------
        // Apply the configuration to the camera
        // -----------------------------------------------------
        camera->configure(config.get());
        std::cout << "Hi res configuration is: " << hi_res_config.toString() << std::endl;
        std::cout << "lo res configuration is: " << lo_res_config.toString() << std::endl;

        // -----------------------------------------------------
        // Get a buffer allocator for the camera
        // -----------------------------------------------------
        allocator = std::make_unique<FrameBufferAllocator>(camera);

        // -----------------------------------------------------
        // Use the generated buffer counts and select the
        // minimum count from the 2 streams
        // -----------------------------------------------------
        size_t n_buffers = INT_MAX;
        for (StreamConfiguration &cfg : *config) {
            ret = allocator->allocate(cfg.stream());
            if (ret < 0) {
                std::cerr << "Can't allocate buffers" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            size_t allocated = allocator->buffers(cfg.stream()).size();
            if (allocated < n_buffers) {
                n_buffers = allocated;
            }
        }
        std::cout << "Allocated " << n_buffers << " buffers for stream" << std::endl;

        // -----------------------------------------------------
        // Create the hi-res-stream from the stream config
        // -----------------------------------------------------
        hi_res_stream = std::make_unique<Stream*>(hi_res_config.stream());
        const std::vector<std::unique_ptr<FrameBuffer>> &hi_res_buffers = allocator->buffers(*hi_res_stream);

        // -----------------------------------------------------
        // Create the lo-res-stream from the stream config
        // -----------------------------------------------------
        lo_res_stream = std::make_unique<Stream*>(lo_res_config.stream());
        const std::vector<std::unique_ptr<FrameBuffer>> &lo_res_buffers = allocator->buffers(*lo_res_stream);

        // -----------------------------------------------------
        // Set the cookie to be the address of the Picamera3Actor
        // object. This will allow the static callback function
        // to access the camera configuration
        // -----------------------------------------------------
        auto cookie = std::bit_cast<u_int64_t>(this);

        // -----------------------------------------------------
        // Create a set of requests and add:
        // - a buffer for the hi-res-stream
        // - a buffer for the lo-res-stream
        // -----------------------------------------------------
        for (unsigned int i = 0; i < n_buffers; ++i) {
            std::unique_ptr<Request> request = camera->createRequest(cookie);
            if (!request) {
                std::cerr << "Can't create request" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const std::unique_ptr<FrameBuffer> &hi_res_buffer = hi_res_buffers[i];
            ret = request->addBuffer(*hi_res_stream, hi_res_buffer.get(), nullptr);
            if (ret < 0 ) {
                std::cerr << "Can't set hi res buffer for request" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            const std::unique_ptr<FrameBuffer> &lo_res_buffer = lo_res_buffers[i];
            ret = request->addBuffer(*lo_res_stream, lo_res_buffer.get(), nullptr);
            if (ret < 0) {
                std::cerr << "Can't set lo res buffer for request" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            // -----------------------------------------------------
            // Add the request to the requests map using the
            // address of the request as the key
            // -----------------------------------------------------
            auto req_adr = std::bit_cast<u_int64_t>(request.get());
            std::cerr << "Request " << req_adr << std::endl;
            requests[req_adr] = std::move(request);
        }

        camera->requestCompleted.connect(Picamera3Actor::request_completed);
        std::cout << "Init camera - complete" << std::endl;
    }


    asio::awaitable<void> Picamera3Actor::start() {
        std::cout << "Start camera " << std::endl;

        camera->start();

        for (const auto& [key, val] : requests) {
            std::cout << key << ": " << val << std::endl;
            camera->queueRequest(val.get());
        }

        while (true) {
            auto [ec,msg] =
                co_await mailbox.async_receive(asio::as_tuple(asio::use_awaitable));
            if (!ec) {
                auto visitor = overload {
                    [this](RecycleReference r_ref) { recycle_request(r_ref.id); },
                };
            }
        }
    }

    void Picamera3Actor::stop() {}

    void Picamera3Actor::recycle_request(uint64_t id) {
        std::cout << "Recycle request : " << id << std::endl;

        auto it = requests.find(id);
        if (it != requests.end()) {
            std::cout << "Found request" << std::endl;
            const auto req = it->second.get();
            req->reuse(Request::ReuseBuffers);
            camera->queueRequest(req);
        }
    }

    // The callback function provided to the camera
    void Picamera3Actor::request_completed(Request *request) {
        auto req_adr = std::bit_cast<uint64_t>(request);

        std::cout << "Request complete " << req_adr << std::endl;

        if (request->status() != Request::RequestCancelled) {
            const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

            // The cookie contains a reference to the Picamera3 object
            uint64_t cookie = request->cookie();
            auto camera = std::bit_cast<Picamera3Actor*>(cookie);

            // Get an image reference for the hi-res image
            const FrameBuffer* hi_res_buffer = buffers.at(*camera->hi_res_stream);
            auto hi_res_img_ref = ImageReference(hi_res_buffer->planes()[0]);

            // Get an image reference for the lo-res image
            const FrameBuffer* lo_res_buffer = buffers.at(*camera->lo_res_stream);
            auto lo_res_img_ref = ImageReference(lo_res_buffer->planes()[0]);

            // create the capture ref
            auto capture_ref = CaptureReference(
                req_adr,
                0,
                hi_res_img_ref,
                lo_res_img_ref );

            camera->capture_ref_actor->post_sync(capture_ref);

            //auto res = camera->cap_ref_chan->try_send(boost::system::error_code{}, capture_ref);
            //if (res == false) {
            //    std::cout << "Failed to send the CaptureReference " << std::endl;
            //}
        }
    }

}
