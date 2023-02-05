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

int FileManager::list_directory(std::string &path, std::string &out)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                out += std::string(ent->d_name) + ""; 
                // get creation time
                // get last modified time
                out += " "; 
                
                if (ent->d_name != NULL) {
                    out += ", ";
                }
            }
        }
        closedir (dir);
    } else {
        return -1;
    }

    return 0;
} 