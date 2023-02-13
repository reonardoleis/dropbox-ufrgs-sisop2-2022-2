#include "sync_controller.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include "../../../commons/ui/ui_template.hpp"
SyncController::SyncController(FileManager server_file_manager)
{
    this->file_manager = server_file_manager;
}

int SyncController::sync_dir(std::string &path) {
    std::string p = std::string("/sync_dir_"+path);
    return this->file_manager.create_directory(p);
}

int SyncController::list_sync_dir(std::string &username, std::string &out) {
    cli_logger log = cli_logger(frontend.get_log_stream());
    char cCurrentPath[FILENAME_MAX];
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        log.set("Failed to get running directory: ERRNO " + std::to_string(errno)).stamp().error();
        return -1;
    }
    std::string b = std::string(cCurrentPath);
    std::string p = std::string(b + "/sync_directories/sync_dir_" + username);

    std::string ref_p = p, ref_out = out;
    log.set("listing directory " + p + out).stamp().info();
    int ret = this->file_manager.list_directory(ref_p, ref_out);
    log.set("listing directory " + p + ref_out).stamp().info();
    return ret;
}
