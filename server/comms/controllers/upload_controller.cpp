#include "upload_controller.hpp"

int UploadController::upload(File file, std::string username) {
    std::string path = "./sync_directories/sync_dir_" + username;
    int err = file.write_file(path);
    return err;
}
