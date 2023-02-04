#include "upload_controller.hpp"

int UploadController::upload(File file, std::string username) {
    std::string path = "sync_dir_" + username;
    file.write_file(path);
    return 0;
}
