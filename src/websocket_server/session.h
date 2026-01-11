#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <algorithm>
#include <memory>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {

public:
    explicit Session(boost::asio::ip::tcp::socket&& socket) : ws(std::move(socket)) {}

    void run();
    void on_run();
    void on_accept(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec,std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

private:
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;
};
