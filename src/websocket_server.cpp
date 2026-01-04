#include "websocket_server.h"

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace io::naturesense {

    asio::awaitable<void> WebsocketServer::actor(asio::io_context& ioc, const char* address, unsigned short port) {
        auto wss = WebsocketServer(ioc, address, port);
        co_await wss.start();
    }

    WebsocketServer::WebsocketServer(asio::io_context& ioc, const char* address, unsigned short port) :
        ioc(ioc) {
        std::cout << "WebsocketServer::WebsocketServer " << address << " " << port << std::endl;

        this->address = net::ip::make_address(address);
        this->port = port;
        listener = std::make_shared<Listener>(ioc, tcp::endpoint{this->address, this->port});
    }

    asio::awaitable<void> WebsocketServer::start() {
        std::cout << "WebsocketServer::start" << std::endl;

        co_await listener->run();
    }

	WebsocketServer::WebsocketServer()

}
