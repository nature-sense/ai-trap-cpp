#include "session_model.h"
#include "sessions.pb.h"

namespace io::naturesense {
    SessionModel::SessionModel(const std::string& session, int detections) {
        this->session = session;
        this->detections = detections;

    }

    std::vector<std::byte> SessionModel::to_protobuf() const {
        Session session_msg;
        session_msg.set_session(session);
        session_msg.set_detections(detections);

        long length = session_msg.ByteSizeLong();
        auto msg = std::vector<std::byte>(length);

        session_msg.SerializeToArray(msg.data(), length);
        return msg;
    }


}