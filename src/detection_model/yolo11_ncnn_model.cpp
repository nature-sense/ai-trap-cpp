//
// Created by steve on 14/12/2025.
//
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>
#include "yolo11_ncnn_model.h"

namespace io::naturesense {
    asio::awaitable<void>  Yolo11NcnnModel::actor(
        Channel<CaptureReference> *det_model_in_chan,
        Channel<CaptureReference> *tracking_in_chan,
        Channel<uint64_t> *recycle_chan,
        cv::Size hi_res_size,
        cv::Size lo_res_size,
        const char* proto_path,
        const char* model_path
        )
    {
        std::cout << "Starting Yolo11NcnnModel actor..." << std::endl;
        auto model = Yolo11NcnnModel(det_model_in_chan, tracking_in_chan, recycle_chan, hi_res_size, lo_res_size, proto_path, model_path);
        co_await model.start();
    }

    Yolo11NcnnModel::Yolo11NcnnModel(
            Channel<CaptureReference> *det_model_in_chan,
            Channel<CaptureReference> *tracking_in_chan,
            Channel<uint64_t> *recycle_chan,
            cv::Size hi_res_size,
            cv::Size lo_res_size,
            const char* proto_path,
            const char* model_path )
    {
        this->det_model_in_chan = det_model_in_chan;
        this->tracking_in_chan = tracking_in_chan;
        this->recycle_chan = recycle_chan;

        this->hi_res_size = hi_res_size;
        this->lo_res_size = lo_res_size;

        net = std::make_unique<ncnn::Net>();
        if (net->load_param(proto_path) < 0 ) {
            std::cerr << "failed to open file " << proto_path << std::endl;
            std::abort();
        }
        if (net->load_model(model_path) <0 ) {
            std::cerr << "failed to open file " << model_path << std::endl;
            std::abort();
        }
    }

    Yolo11NcnnModel::~Yolo11NcnnModel() = default;

    asio::awaitable<void> Yolo11NcnnModel::start() const {
        // Initialise the model

        while (true) {
            auto ref = co_await det_model_in_chan->async_receive(asio::use_awaitable);

            std::cout << "Received reference : " << ref.get_id() << std::endl;

            ImageReference hi_res_image = ref.get_hi_res_image();
            ImageReference lo_res_image = ref.get_lo_res_image();

            hi_res_image.trace();
            lo_res_image.trace();

            // get pointers to the dma data
            std::pair<uint8_t*, uint32_t> hi_res_img_data = hi_res_image.map_data();
            //std::pair<uint8_t*, uint32_t> lo_res_img_data = hi_res_image.map_data();

            cv::Mat hi_res_matrix(lo_res_size.height, lo_res_size.width, CV_8UC1, hi_res_img_data.first);

            co_await recycle_chan->async_send(boost::system::error_code{}, ref.get_id(), asio::use_awaitable);
        }
    }
}
