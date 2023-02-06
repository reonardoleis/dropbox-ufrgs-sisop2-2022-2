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
    std::string p = std::string("./sync_directories/sync_dir_"+username+"/");
    cli_logger log = cli_logger(frontend.get_log_stream());

    log.set("listing directory " + p).stamp().info();
   
    return this->file_manager.list_directory(p, out);
}
