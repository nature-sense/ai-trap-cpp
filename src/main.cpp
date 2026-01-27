
//
// Created by steve on 09/12/2025.
//

#include "references/capture_reference/capture_reference.h"

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>

#include "detection_camera/picamera3/picamera3_actor.h"
#include "detection_model/yolo11_ncnn/yolo11_ncnn_actor.h"
#include "actor_framework/actor.h"
//#include "session_database/session_database.h"

using namespace io::naturesense;

namespace asio = boost::asio;
namespace sys = boost::system;

int main() {

    // Create the io_context and give it something to do
    asio::io_context ioc;
    auto work = boost::asio::make_work_guard(ioc);

    toml::table config;
    try
    {
       config = toml::parse_file("/home/steve/ai-trap-cpp/system.toml");
        //std::cout << config << "\n";
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Parsing failed:" << err;
        std::exit(-1);
    }

    // ---------------------------------------------------------------
    // Create the actors
    // ---------------------------------------------------------------
    Picamera3Actor picam_actor = Picamera3Actor(&ioc, config);
    Yolo11NcnnActor yolo11_actor = Yolo11NcnnActor(&ioc, config);

    // ---------------------------------------------------------------
    // Link the actors
    // ---------------------------------------------------------------
    picam_actor.link_actor(&yolo11_actor);
    yolo11_actor.link_actor(&picam_actor);

    // ---------------------------------------------------------------
    // Initialise the actors
    // ---------------------------------------------------------------
    picam_actor.initialise();
    yolo11_actor.initialise();

    ioc.run();;

    return 0;
}
