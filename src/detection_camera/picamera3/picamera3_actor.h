#pragma once
#include <boost/asio.hpp>
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/awaitable.hpp>
#include "actor_framework/actor.h"

#include "references/capture_reference/capture_reference_mixin.h"
#include "references/recycle_reference/recycle_reference.h"
#include "references/recycle_reference/recycle_reference_mixin.h"

namespace asio = boost::asio;
using asio::experimental::concurrent_channel;

namespace io::naturesense {

    // Define the mailbox channel type
    using boost::system::error_code;

    class Picamera3Actor : public Actor, public RecycleReferenceMixin {
        using Mailbox = concurrent_channel<void(error_code, std::variant<RecycleReference>)>;

    public:
        Picamera3Actor(asio::io_context* ioc, toml::table& cfg) : Actor(ioc, cfg), RecycleReferenceMixin() {};
        ~Picamera3Actor() override;

        // ----------------------------------------------------------
        // Actor function implementations
        // ----------------------------------------------------------
        void initialise() override;
        asio::awaitable<void> start() override;
        void stop() override;

        // ----------------------------------------------------------
        // RecycleReferenceMixin function implementations
        // ----------------------------------------------------------
        asio::awaitable<void> post_async(RecycleReference ref) override;
        void post_sync(RecycleReference ref) override;

        // ----------------------------------------------------------
        // Link to a CaptureReference actor
        // ----------------------------------------------------------
        void link_actor(CaptureReferenceMixin* actor);

    private:
        void recycle_request(uint64_t id);
        static void request_completed(Request *request);

        std::string name = "picamera3";

        cv::Size hi_res_size;
        cv::Size lo_res_size;

        Mailbox mailbox{ioc->get_executor(), 10};
        CaptureReferenceMixin* capture_ref_actor = nullptr; // target

        std::unique_ptr<Stream*> hi_res_stream;
        std::unique_ptr<Stream*> lo_res_stream;

        std::unique_ptr<CameraManager> cm;
        std::shared_ptr<Camera> camera;

        std::map<u_int64_t, std::unique_ptr<Request>> requests;
        std::mutex mtx;
        std::unique_ptr<FrameBufferAllocator> allocator;
    };
}