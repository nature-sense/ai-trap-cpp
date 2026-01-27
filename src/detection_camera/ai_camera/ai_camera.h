#pragma once

#pragma once
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <boost/asio/awaitable.hpp>

#include <memory>
#include "../../references/capture_reference/capture_reference.h"
#include "channels/channel.h"

using namespace libcamera;

namespace io::naturesense {
    class AiCamera {
    public:
        static asio::awaitable<void> actor(
            Channel<CaptureReference> *tracking_in_chan,
            Channel<uint64_t> *recycle_chan,
            cv::Size hi_res_size,
            cv::Size lo_res_size,
            const char* network_file
            );

    private:
        AiCamera(
            Channel<CaptureReference> *tracking_in_chan,
            Channel<uint64_t> *recycle_chan
           );

        ~AiCamera();

        int init(cv::Size hi_res_size, cv::Size lo_res_size, const char* network_file);

        void recycle_request(uint64_t id);

        [[nodiscard]] asio::awaitable<void> start();
        void stop();

        static void request_completed(Request *request);

        int device_fd;

        Rectangle full_sensor_resolution_ = Rectangle(0, 0, 4056, 3040);

        std::unique_ptr<Stream*> hi_res_stream;
        std::unique_ptr<Stream*> lo_res_stream;

        Channel<CaptureReference> *tracking_in_chan;
        Channel<uint64_t> *recycle_chan;

        std::unique_ptr<CameraManager> cm;
        std::shared_ptr<Camera> camera;

        std::map<u_int64_t, std::unique_ptr<Request>> requests;
        std::mutex mtx;

        std::unique_ptr<FrameBufferAllocator> allocator;
    };;
}