//
// Created by steve on 14/12/2025.
//
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/system/error_code.hpp>

#include "yolo11_ncnn_actor.h"
#include "actor_framework/mailbox.h"

namespace io::naturesense {

    Yolo11NcnnActor::~Yolo11NcnnActor() = default;

    awaitable<void> Yolo11NcnnActor::post_async(CaptureReference ref) {
        co_await mailbox.async_send(boost::system::error_code(), ref, asio::use_awaitable);
    }
    void Yolo11NcnnActor::post_sync(CaptureReference ref) {
        mailbox.try_send(error_code(), ref);
    }

    // -------------------------------------------------------------
    // Link to the next CaptureReference actor in the pipeline
    // -------------------------------------------------------------
    void Yolo11NcnnActor::link_actor(CaptureReferenceMixin* actor) {
        this->capture_ref_actor = actor;
    }

    void Yolo11NcnnActor::link_actor(RecycleReferenceMixin* actor) {
        this->recycle_ref_actor = actor;
    }

    void Yolo11NcnnActor::initialise() {

        std::cout << "Initialising Yolo11NcnnActor" << std::endl;

        // -----------------------------------------------------
        // Read the frame sizes from the system.toml file
        // -----------------------------------------------------
        auto conf = this->cfg[name];

        const char* proto_path = this->cfg["yolo11ncnn"]["proto_path"].value_or("undefined");
        const char* model_path = this->cfg["yolo11ncnn"]["model_path"].value_or("undefined");

        int hi_res_width  = this->cfg["hi_res_size"]["width"].value_or(0);
        int hi_res_height  = this->cfg["hi_res_size"]["height"].value_or(0);
        this->hi_res_size = cv::Size(hi_res_width, hi_res_height);

        net = std::make_unique<ncnn::Net>();
        if (net->load_param(proto_path) < 0 ) {
            std::cerr << "failed to open file " << proto_path << std::endl;
            std::abort();
        }
        if (net->load_model(model_path) <0 ) {
            std::cerr << "failed to open file " << model_path << std::endl;
            std::abort();
        }

        std::cout << "Init Yolo11NcnnActor - complete" << std::endl;
    }


    awaitable<void> Yolo11NcnnActor::start() {
        // Initialise the model

        while (true) {
            auto [ec,msg] =
                co_await mailbox.async_receive(asio::as_tuple(boost::asio::use_awaitable));
            if (!ec) {
                auto visitor = overload {
                    [this](const CaptureReference& ref) {
                        std::cout << "Received reference : " << ref.id << std::endl;

                        ImageReference hi_res_image = ref.hi_res_image;
                        ImageReference lo_res_image = ref.lo_res_image;

                        hi_res_image.trace();
                        lo_res_image.trace();

                        // get pointers to the dma data
                        std::pair<uint8_t*, uint32_t> hi_res_img_data = hi_res_image.map_data();
                        //std::pair<uint8_t*, uint32_t> lo_res_img_data = hi_res_image.map_data();

                        cv::Mat hi_res_matrix(lo_res_size.height, lo_res_size.width, CV_8UC1, hi_res_img_data.first);

                    },
                };
            }
        }
    }

    void Yolo11NcnnActor::stop() {};

}
