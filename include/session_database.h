#pragma once
#include <rocksdb/db.h>
#include <iostream>
#include <boost/asio/awaitable.hpp>

namespace asio = boost::asio;
using namespace rocksdb;

namespace io::naturesense {
    class SessionDatabase {

    public:

		static asio::awaitable<void> actor(const std::string& db_path);

        SessionDatabase(const std::string& db_path);
        ~SessionDatabase();

        void add_session(const std::string& sesson);
        void add_detection_to_session(const std::string& session void* detection);

    private:

        DB* db;
        Options options;
        std::vector<ColumnFamilyHandle*> handles;
        std::vector<ColumnFamilyDescriptor> cf_descriptors;
    	std::map<std::string, ColumnFamilyHandle*> session_handles;

    };
}