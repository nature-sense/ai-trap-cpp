
//
// Created by steve on 09/12/2025.
//

#include <thread> // For std::this_thread::sleep_for
//#include "../include/disabled/capture_reference.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/co_spawn.hpp>
//#include <opencv2/opencv.hpp>

#include "include/channel.h"
//#include "include/detection_camera.h"
//#include "include/yolo11_ncnn_model.h"
#include "websocket_server.h"

using namespace io::naturesense;

namespace asio = boost::asio;
namespace sys = boost::system;
//namespace exp = asio::experimental;
//8096

int main() {

    const char* proto_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.param";
    const char* model_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.bin";

    //asio::thread_pool pool(4);

    //cv::Size hi_res_size(2304, 1296);
    //cv::Size lo_res_size(640,640);

    // Createv the io_context and give it something to do
    asio::io_context ioc;
    auto work = boost::asio::make_work_guard(ioc);

    //Channel<CaptureReference> cap_ref_chan {ctx.get_executor(), 10};
    //Channel<uint64_t> recycle_chan {ctx.get_executor(), 10};

    /*asio::co_spawn(
        ctx,
        DetectionCamera::actor(&cap_ref_chan, &recycle_chan, hi_res_size, lo_res_size),
        asio::detached
    );*/

    /*asio::co_spawn(
        ctx,
        Yolo11NcnnModel::actor(&cap_ref_chan, &recycle_chan,  hi_res_size, lo_res_size, proto_path, model_path),
        asio::detached
    );*/


	asio::co_spawn(
        ioc,
		WebsocketServer::actor(ioc, "0.0.0.0", 8096),
        asio::detached
    );


    ioc.run();;

    return 0;
}
