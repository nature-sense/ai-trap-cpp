#pragma once
#include <string>
#include "sessions.pb.h"

namespace io::naturesense {
    class SessionModel {

    public:
        SessionModel(std::string session, int detections);
        [[nodiscard]] std::vector<std::byte> to_protobuf() const;

        const std::string_view session;
        int detections;
    };
}