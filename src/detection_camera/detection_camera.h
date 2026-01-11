#pragma once
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <boost/asio/awaitable.hpp>
#include <queue>

#include <memory>
#include "../references/capture_reference.h"
#include "../channels/channel.h"

using namespace libcamera;

namespace io::naturesense {
    struct ImageSize {
        int width;
        int height;
    };

    class DetectionCamera {
    public:
        static asio::awaitable<void> actor(
            Channel<CaptureReference> *cap_ref_chan,
            Channel<uint64_t> *recycle_chan,
            cv::Size hi_res_size,
            cv::Size lo_res_size );

    private:
        DetectionCamera(
            Channel<CaptureReference> *cap_ref_chan,
            Channel<uint64_t> *recycle_chan );

        ~DetectionCamera();
        int init(cv::Size hi_res_size, cv::Size lo_res_size);

        void recycle_request(uint64_t id);

        [[nodiscard]] asio::awaitable<void> start();
        void stop();

        static void request_completed(Request *request);

        std::unique_ptr<Stream*> hi_res_stream;
        std::unique_ptr<Stream*> lo_res_stream;

        Channel<CaptureReference> *cap_ref_chan;
        Channel<uint64_t> *recycle_chan;

        std::unique_ptr<CameraManager> cm;
        std::shared_ptr<Camera> camera;

        std::map<u_int64_t, std::unique_ptr<Request>> requests;

        //std::shared_ptr<std::queue<std::unique_ptr<Request>>> requests;
        std::mutex mtx;

        std::unique_ptr<FrameBufferAllocator> allocator;
    };

}
