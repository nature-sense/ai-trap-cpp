#pragma once
#include <iostream>
#include <boost/asio/awaitable.hpp>
#include "session_model.h"
#include "channels/channel.h"
#include "websocket_server/protocol_msg.h"
#include <filesystem>

//"session.open"
//"session.close"
//"session.all"
//"session.detections"


namespace asio = boost::asio;
namespace fs = std::filesystem;

namespace io::naturesense {
    class SessionDatabase {

    public:

		static asio::awaitable<void> actor(const std::string& root, Channel<ProtocolMsg>* sessions_chan);
        SessionDatabase(const std::string& root, Channel<ProtocolMsg>* sessions_chan);
        [[nodiscard]] asio::awaitable<void> start();

        void open_session_cmd(std::string& protobuf);
        void close_session_cmd(std::string& protobuf);
        void all_sessions_cmd(std::string& protobuf);
        void session_detections_cmd(std::string& protobuf);

        void add_session(const std::string& session) const;
        [[nodiscard]] std::vector<std::string> get_sessions() const;

    private:
        [[nodiscard]] int count_detections(const std::string& session) const;

        fs::path dir_path;
        Channel<ProtocolMsg>* sessions_chan;
        //std::unordered_map<std::string, std::function<std::string&()>> commands{};
    };
}