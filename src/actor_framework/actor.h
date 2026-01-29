#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "toml++/toml.h"

namespace io::naturesense {
    class Actor {
    public:

        Actor(boost::asio::io_context* ioc, toml::table& cfg) : ioc(ioc), cfg(cfg) {}
        virtual ~Actor() = default;;
        virtual void initialise() = 0;
        virtual boost::asio::awaitable<void> start() = 0;
        virtual void stop() = 0;

    protected:
        std::string name = "actor";
        boost::asio::io_context* ioc;
        toml::table& cfg;
    };
}


