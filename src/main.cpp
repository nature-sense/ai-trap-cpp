
//
// Created by steve on 09/12/2025.
//

#include <thread> // For std::this_thread::sleep_for
#include "references/capture_reference.h"

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/co_spawn.hpp>
#include <opencv2/opencv.hpp>

#include "channels/channel.h"
#include "detection_camera/detection_camera.h"
#include "detection_model//yolo11_ncnn_model.h"
#include "session_database/session_database.h"
#include "websocket_server/websocket_server.h"
#include "websocket_server/protocol_msg.h"

using namespace io::naturesense;

namespace asio = boost::asio;
namespace sys = boost::system;
//namespace exp = asio::experimental;
//8096

int main() {

    const char* proto_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.param";
    const char* model_path = "/home/steve/ai-trap-cpp/resources/model.ncnn.bin";

    //asio::thread_pool pool(4);

    cv::Size hi_res_size(2304, 1296);
    cv::Size lo_res_size(640,640);

    // Createv the io_context and give it something to do
    asio::io_context ioc;
    auto work = boost::asio::make_work_guard(ioc);

    // =================================================================
    // Recycle Channel - passes used capture references from the
    // Detection-Model  to the Detection-Camera to be reused
    // =================================================================
    Channel<uint64_t> recycle_chan {ioc.get_executor(), 10};

    // =================================================================
    // Detection Model Input Channel - passes captured images from the
    // Detection-Camera to the Detection-Model
    // =================================================================
    Channel<CaptureReference> det_model_in_chan {ioc.get_executor(), 10};

    // =================================================================
    // Tracking Input Channel - passes captured images (with detection
    // references)from the Detection Model to the tracker
    // =================================================================
    Channel<CaptureReference> tracking_in_chan {ioc.get_executor(), 10};

    // =================================================================
    // Extractor Input Channel - passes captured images (with detection
    // references and track-ids) from the tracker to the extractor
    // =================================================================
    Channel<CaptureReference> extractor_in_chan {ioc.get_executor(), 10};

    // =================================================================
    // sessions_chan - received protocol buffer session messages
    // =================================================================
    Channel<ProtocolMsg> sessions_chan {ioc.get_executor(), 10};

    // =================================================================
    // websocket_tx_chan - protocol buffer messages to be transmitted
    // =================================================================
    Channel<ProtocolMsg> websocket_tx_chan {ioc.get_executor(), 10};

    // =================================================================
    // Session database actor
    // =================================================================
    asio::co_spawn(
            ioc,
            SessionDatabase::actor("", &sessions_chan),
            asio::detached
        );

    // =================================================================
    // Detection camera actor
    // =================================================================
    asio::co_spawn(
        ioc,
        DetectionCamera::actor(&det_model_in_chan, &recycle_chan, hi_res_size, lo_res_size),
        asio::detached
    );

    // =================================================================
    // Detection model actor
    // =================================================================
    asio::co_spawn(
        ioc,
        Yolo11NcnnModel::actor(&det_model_in_chan, &tracking_in_chan, &recycle_chan,  hi_res_size, lo_res_size, proto_path, model_path),
        asio::detached
    );

    // ================================================================
    // Websocket server actor
    // =================================================================
	asio::co_spawn(
        ioc,
		WebsocketServer::actor(ioc, "0.0.0.0", 8096),
        asio::detached
    );


    ioc.run();;

    return 0;
}
