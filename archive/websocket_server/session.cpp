#include "session.h"
#include <iostream>

// Get on the correct executor
void Session::run() {

    std::cout << "Session::run " << std::endl;

    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(ws.get_executor(),
        beast::bind_front_handler(
            &Session::on_run,
            shared_from_this()));
}

// Start the asynchronous operation
void Session::on_run() {
    // Set suggested timeout settings for the websocket
    ws.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server)
	);

    // Set a decorator to change the Server of the handshake
    ws.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-server-async");
        }));
    // Accept the websocket handshake
    ws.async_accept(
        beast::bind_front_handler(
            &Session::on_accept,
            shared_from_this()));
}

void Session::on_accept(beast::error_code ec) {
    if(ec)
        return
        //return fail(ec, "accept");

    // Read a message
    do_read();
}

void Session::do_read() {
    // Read a message into our buffer
    ws.async_read(
        buffer,
        beast::bind_front_handler(
            &Session::on_read,
            shared_from_this()));
}

void Session::on_read(beast::error_code ec,std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if(ec == websocket::error::closed)
        return;

    if(ec)
        return;
        //return fail(ec, "read");

    // Echo the message
    ws.text(ws.got_text());
    ws.async_write(
        buffer.data(),
        beast::bind_front_handler(
            &Session::on_write,
            shared_from_this()));
}

void Session::on_write( beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return;
        //return fail(ec, "write");

    // Clear the buffer
    buffer.consume(buffer.size());

    // Do another read
    do_read();
}

