#pragma once

#include <string>

namespace io::naturesense {
    class ProtocolMsg {
    public:
        ProtocolMsg() : identifier(""), protobuf("") {};
        ProtocolMsg(std::string identifier, std::string protobuf);
        std::string identifier;
        std::string protobuf;
    };
}