//
// Created by steve on 02/12/2025.
//
#include <libcamera/libcamera.h>
#include <climits>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
//#include <boost/fiber/all.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>
#include "detection_camera.h"
#include "capture_reference.h"
#include "channel.h"

using namespace libcamera;

namespace io::naturesense {
    asio::awaitable<void> DetectionCamera::actor(
        Channel<CaptureReference> *cap_ref_chan,
        Channel<uint64_t> *recycle_chan,
        cv::Size hi_res_size,
        cv::Size lo_res_size ){

        std::cout << "Starting DetectionCamera actor..." << std::endl;

        auto camera = DetectionCamera(cap_ref_chan, recycle_chan);
        camera.init(hi_res_size, lo_res_size);
        co_await camera.start();
    }

    DetectionCamera::DetectionCamera(Channel<CaptureReference> *cap_ref_chan, Channel<uint64_t> *recycle_chan) {
        this->cap_ref_chan = cap_ref_chan;
        this->recycle_chan = recycle_chan;

        cm = std::make_unique<CameraManager>();
        cm->start();
    }

    DetectionCamera::~DetectionCamera() {
        cm->stop();
    }

    int DetectionCamera::init(cv::Size hi_res_size, cv::Size lo_res_size) {
        std::cout << "Init camera" << std::endl;

        int ret;
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

        camera->requestCompleted.connect(DetectionCamera::request_completed);
        std::cout << "Init camera - complete" << std::endl;

        return 0;
    }

    asio::awaitable<void> DetectionCamera::start() {
        std::cout << "Start camera " << std::endl;

        camera->start();

        for (const auto& [key, val] : requests) {
            std::cout << key << ": " << val << std::endl;
            camera->queueRequest(val.get());
        }

        while (true) {
            auto id = co_await recycle_chan->async_receive(asio::use_awaitable);
            std::cout << "Recycle = " << id << std::endl;
            this->recycle_request(id);
        }
    }

    void DetectionCamera::stop() {
        camera->stop();
        allocator->free(*hi_res_stream);
        allocator->free(*lo_res_stream);
        camera->release();
        camera.reset();
    }

    void DetectionCamera::recycle_request(uint64_t id) {
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
    void DetectionCamera::request_completed(Request *request) {
        auto req_adr = std::bit_cast<u_int64_t>(request);

        std::cout << "Request complete " << req_adr << std::endl;

        //const Stream* hi_res_s = DetectionCamera::hi_res_stream;

        // Start s boost fiber to handle the event
        //boost::fibers::fiber handle([&] { ;
        if (request->status() != Request::RequestCancelled) {
            const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

            // The cookie contains a reference to the DetectionCamera object
            uint64_t cookie = request->cookie();
            auto camera = std::bit_cast<DetectionCamera*>(cookie);

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

            auto res = camera->cap_ref_chan->try_send(boost::system::error_code{}, capture_ref);
            if (res == false) {
                std::cout << "Failed to send the CaptureReference " << std::endl;
            }
        }
    }
}


