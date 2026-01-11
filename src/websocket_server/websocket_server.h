#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>

#include "listener.h"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

namespace io::naturesense {
    class WebsocketServer {

    public:
        static asio::awaitable<void> actor(net::io_context& ioc, const char* address, unsigned short port);
        WebsocketServer(asio::io_context& ioc, const char* address, unsigned short port);
        [[nodiscard]] asio::awaitable<void> start() const;

    private:
        asio::io_context& ioc;

        net::ip::address address;
        unsigned short port;
        std::shared_ptr<Listener> listener;
    };
}



