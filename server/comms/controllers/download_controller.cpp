#include "download_controller.hpp"

int DownloadController::download(File *out, std::string filename, std::string username) {
    std::string path = "./sync_directories/sync_dir_" + username + "/" + filename;
    out = new File(path);
    if (out->read_file() < 0) {
        return -1;
    }

    return 0;
}