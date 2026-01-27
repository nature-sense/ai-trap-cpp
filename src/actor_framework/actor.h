#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "toml++/toml.h"

namespace asio = boost::asio;

namespace io::naturesense {
    class Actor {
    public:

        Actor(asio::io_context* ioc, toml::table& cfg) : ioc(ioc), cfg(cfg) {}
        virtual ~Actor() {};
        virtual void initialise() = 0;
        virtual asio::awaitable<void> start() = 0;
        virtual void stop() = 0;

    protected:
        std::string name = "actor";
        asio::io_context* ioc;
        toml::table& cfg;
    };
}


