#include "listener.h"
#include "session.h"
#include <iostream>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/co_spawn.hpp>

namespace asio = boost::asio;
namespace sys = boost::system;

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint) : ioc(ioc), acceptor(ioc) {

    std::cout << "Listener::Listener " << std::endl;

    beast::error_code ec;

    // Open the acceptor
    std::cout << "Listener Opening acceptor " << std::endl;
    acceptor.open(endpoint.protocol(), ec);
    if(ec) {
        //fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if(ec) {
        //fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    std::cout << "Listener binding acceptor " << std::endl;
    acceptor.bind(endpoint, ec);
    if(ec) {
        //fail(ec, "bind");
        return;
    }
}

// Start accepting incoming connections
boost::asio::awaitable<void> Listener::run() {
    beast::error_code ec;

    std::cout << "Listener::run" << std::endl;

 // Start listening for connections
    std::cout << "Listening for connections " << std::endl;
    acceptor.listen(net::socket_base::max_listen_connections, ec);
    if(ec) {
    	std::cerr << "Listening for connections error " << ec << std::endl;
        co_return;
    }

	while (true) {
        boost::asio::ip::tcp::socket socket =
			co_await acceptor.async_accept(asio::use_awaitable);
    	std::cout << "Connection request " << std::endl;
        asio::co_spawn(
			acceptor.get_executor(),
			this->on_accept(std::move(socket)),
			asio::detached
		);
	}
}

asio::awaitable<void> Listener::on_accept(tcp::socket socket) {
    std::cout << "Request accepted " << std::endl;
    std::make_shared<Session>(std::move(socket))->run();
	co_return;
}
