#include "session_database.h"


using namespace rocksdb;

namespace io::naturesense {


	asio::awaitable<void> SessionDatabase::actor(const std::string& db_path) {

	}

    SessionDatabase::SessionDatabase(const std::string& db_path) {
        Status s;
		std::vector<std::string> column_family_names;

        options.create_if_missing = true;
        options.create_missing_column_families = true;

        // get the column family names i.i sessiond
        s = DB::ListColumnFamilies(options, db_path, &column_family_names);
		// Prepare descriptors for all found column families

		for (const auto& name : column_family_names) {
    		cf_descriptors.emplace_back(ColumnFamilyDescriptor(name, rocksdb::ColumnFamilyOptions()));
		}

        s = DB::Open(options, db_path, cf_descriptors, &handles, &db);
        if (!s.ok()) {
	            std::cerr << "Unable to open DB: " << s.ToString() << std::endl;
            std::exit(EXIT_FAILURE);
        }

		if (column_family_names.size() != handles.size()) {
        	std::cerr << "Error: names and handles vectors have different sizes." << std::endl;
            std::exit(EXIT_FAILURE);
		}
        for (size_t i = 0; i < column_family_names.size(); ++i) {
            session_handles[column_family_names[i]] = *handles[i]; // Use the [] operator for insertion
        }

    }

	SessionDatabase::~SessionDatabase() {
		for(auto handle : handles) {
			delete handle;
		}
		db.close();
	}

   	void SessionDatabase::add_session(const std::string& sesson) {
		ColumnFamilyHandle* cf_handle;
		ColumnFamilyOptions cf_options;

		s = db->CreateColumnFamily(cf_options, session, &cf_handle);

	}
}
