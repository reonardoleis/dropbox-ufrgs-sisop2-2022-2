#include "delete_controller.hpp"

int DeleteController::delete_file(std::string filename, std::string username)
{
    std::string path = "./sync_directories/sync_dir_" + username + "/" + filename;
    if (remove(path.c_str()) < 0) {
        return -1;
    }


    return 0;
}