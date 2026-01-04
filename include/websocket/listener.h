#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/awaitable.hpp>
#include <memory>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(net::io_context& ioc,tcp::endpoint endpoint);

    boost::asio::awaitable<void>  run();
    void do_accept();


private :
    boost::asio::awaitable<void> on_accept(tcp::socket socket);

    net::io_context& ioc;
    tcp::acceptor acceptor;
};