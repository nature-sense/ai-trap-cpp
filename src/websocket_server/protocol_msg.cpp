//
// Created by steve on 07/01/2026.
//

#include "protocol_msg.h"

namespace io::naturesense {

    ProtocolMsg::ProtocolMsg(std::string identifier, std::string protobuf) {
        this->identifier = std::move(identifier);
        this->protobuf = std::move(protobuf);
    }
}
