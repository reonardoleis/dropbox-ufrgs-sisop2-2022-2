#include "sync_controller.hpp"

SyncController::SyncController(FileManager server_file_manager)
{
    this->file_manager = server_file_manager;
}

int SyncController::sync_dir(std::string &path) {
    return this->file_manager.create_directory(path);
}

