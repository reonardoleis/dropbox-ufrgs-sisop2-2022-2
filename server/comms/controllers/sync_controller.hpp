#include <string>
#include "../../../commons/file_manager/file_manager.hpp"

class SyncController {
    private:
        FileManager file_manager;
    public:
        SyncController(FileManager server_file_manager);
        int sync_dir(std::string &path);
};