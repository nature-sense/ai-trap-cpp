#pragma once
#include <boost/asio/awaitable.hpp>
#include "capture_reference.h"
#include "channel.h"
#include <ncnn/net.h>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

namespace io::naturesense {
    class Yolo11NcnnModel {
    public:
        static asio::awaitable<void> actor(
            Channel<CaptureReference> *cap_ref_chan,
            Channel<uint64_t> *recycle_chan,
            cv::Size hi_res_size,
            cv::Size lo_res_size,
            const char* proto_path,
            const char* model_path );

    private:
        Yolo11NcnnModel(
            Channel<CaptureReference> *cap_ref_chan,
            Channel<uint64_t> *recycle_chan,
            cv::Size hi_res_size,
            cv::Size lo_res_size,
            const char* proto_path,
            const char* model_path );

        ~Yolo11NcnnModel();

        [[nodiscard]] asio::awaitable<void> start() const;

        std::unique_ptr<Stream*> hi_res_stream;
        std::unique_ptr<ncnn::Net> net;

        Channel<CaptureReference> *cap_ref_chan;
        Channel<uint64_t> *recycle_chan;

        cv::Size hi_res_size;
        cv::Size lo_res_size;
    };
}
