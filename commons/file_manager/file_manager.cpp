#include "file_manager.hpp"
#include <sys/stat.h>

FileManager::FileManager(){}

int FileManager::create_directory(std::string &path)
{
    mkdir(this->base_path.c_str(), 0777);

    std::string full_path = this->base_path + path;
    return mkdir(full_path.c_str(), 0777);
}

void FileManager::set_base_path(std::string &path)
{
    this->base_path = path;
}