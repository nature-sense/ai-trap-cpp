#pragma once

#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/io_context.hpp>

namespace asio = boost::asio;
namespace sys = boost::system;
namespace ex = asio::experimental;

template <typename Message>
using Channel = ex::concurrent_channel<void(sys::error_code, Message)>;
