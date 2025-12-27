
//
// Created by steve on 09/12/2025.
//

#include <thread> // For std::this_thread::sleep_for
#include "capture_reference.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/co_spawn.hpp>

#include "include/channel.h"
#include "include/detection_camera.h"
#include "include/yolo11_ncnn_model.h"

using namespace io::naturesense;

namespace asio = boost::asio;
namespace sys = boost::system;
//namespace exp = asio::experimental;

int main() {

    const char* proto_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.param";
    const char* model_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.bin";

    //asio::thread_pool pool(4);

    cv::Size hi_res_size(2304, 1296);
    cv::Size lo_res_size(640,640);

    // Createv the io_context and give it something to do
    asio::io_context ctx;
    auto work = boost::asio::make_work_guard(ctx);

    Channel<CaptureReference> cap_ref_chan {ctx.get_executor(), 10};
    Channel<uint64_t> recycle_chan {ctx.get_executor(), 10};

    asio::co_spawn(
        ctx,
        DetectionCamera::actor(&cap_ref_chan, &recycle_chan, hi_res_size, lo_res_size),
        asio::detached
    );

    asio::co_spawn(
        ctx,
        Yolo11NcnnModel::actor(&cap_ref_chan, &recycle_chan,  hi_res_size, lo_res_size, proto_path, model_path),
        asio::detached
    );

    ctx.run();;

    return 0;
}
