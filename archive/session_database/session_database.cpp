#include "session_database.h"

#include <boost/asio/use_awaitable.hpp>

#include "session_model.h"

namespace io::naturesense {
	asio::awaitable<void> SessionDatabase::actor(const std::string& root, Channel<ProtocolMsg>* sessions_chan) {
		std::cout << "Starting SessionDatabase actor...." << std::endl;
		auto sd = SessionDatabase(root, sessions_chan);
		co_await sd.start();
	}

	SessionDatabase::SessionDatabase(const std::string &root, Channel<ProtocolMsg>* sessions_chan) {
		this->dir_path = root;
		this->sessions_chan = sessions_chan;
	}

	[[nodiscard]] asio::awaitable<void> SessionDatabase::start() {

		while (true) {
			ProtocolMsg msg = co_await sessions_chan->async_receive(asio::use_awaitable);
			std::cout << "msg = "  << std::endl;
			if (msg.identifier == "session.open") {
				open_session_cmd(msg.protobuf);
			} else if (msg.identifier == "session.close") {
				close_session_cmd(msg.protobuf);
			} else if (msg.identifier == "sessions.all") {
				all_sessions_cmd(msg.protobuf);
			} else if (msg.identifier == "session.detections") {
				session_detections_cmd(msg.protobuf);
			}
		}
	}

	void SessionDatabase::open_session_cmd(std::string& protobuf) {}
	void SessionDatabase::close_session_cmd(std::string& protobuf) {}
	void SessionDatabase::all_sessions_cmd(std::string& protobuf) {}
	void SessionDatabase::session_detections_cmd(std::string& protobuf) {}


	void SessionDatabase::add_session(const std::string& session) const {

		fs::path session_path = dir_path / session;

		try {
			if (fs::create_directory(session_path)) {
				std::cout << "Directory '" << session_path << "' created successfully" << std::endl;
			} else {
				// This path is taken if the directory already exists
				std::cout << "Directory '" << session_path << "' already exists (or wasn't created for another reason)" << std::endl;
			}
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Filesystem error: " << e.what() << std::endl;
		}
	}

	std::vector<std::string> SessionDatabase::get_sessions() const {
		std::vector<std::string> sessions;
		for (const auto& entry : fs::directory_iterator(dir_path)) {
			sessions.emplace_back(entry.path().filename());
		}
		return sessions;
	}

	int SessionDatabase::count_detections(const std::string& session) const {
		fs::path session_path = dir_path / session;
		int count = 0;
		for (const auto& entry : fs::directory_iterator(dir_path))
			++count;
		return count;

	}







}
