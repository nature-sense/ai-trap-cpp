#pragma once

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/awaitable.hpp>

#include "actor_framework/actor.h"
#include "references/capture_reference/capture_reference.h"
#include "references/capture_reference/capture_reference_mixin.h"

#include "references/recycle_reference/recycle_reference.h"
#include "references/recycle_reference/recycle_reference_mixin.h"

#include <ncnn/net.h>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>


using boost::asio::experimental::concurrent_channel;
using boost::asio::io_context;
using boost::asio::awaitable;

namespace io::naturesense {

    // Define the mailbox channel type
    using boost::system::error_code;

    class Yolo11NcnnActor : public Actor, public CaptureReferenceMixin {
        using Mailbox = concurrent_channel<void(error_code, std::variant<CaptureReference>)>;
    public:
        Yolo11NcnnActor(io_context* ioc,toml::table& cfg) : Actor(ioc, cfg), CaptureReferenceMixin() {};
        ~Yolo11NcnnActor() override;

        void initialise() override;
        awaitable<void> start() override;
        void stop() override;

        // Link to next CaptureReference actor
        void link_actor(CaptureReferenceMixin* actor);

        // Link to the RecycleReference actor
        void link_actor(RecycleReferenceMixin* actor);

        // Functions
        awaitable<void> post_async(CaptureReference ref) override;
        void post_sync(CaptureReference ref) override;

    private:

        std::string name = "yolo11ncnn";

        Mailbox mailbox{ioc->get_executor(), 10};
        CaptureReferenceMixin* capture_ref_actor = nullptr;
        RecycleReferenceMixin* recycle_ref_actor = nullptr;

        //std::unique_ptr<Stream*> hi_res_stream;
        std::unique_ptr<ncnn::Net> net;

        cv::Size hi_res_size;
        cv::Size lo_res_size;
    };
}
