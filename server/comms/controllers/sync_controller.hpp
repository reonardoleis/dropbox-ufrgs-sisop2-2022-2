#pragma once
#include <string>
#include "../../../commons/file_manager/file_manager.hpp"

class SyncController {
    private:
        FileManager file_manager;
    public:
        SyncController(FileManager server_file_manager);
        int sync_dir(std::string &path);
        int list_sync_dir(std::string &username, std::string &out);
        std::string get_sync_dir(std::string &username);
};