#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>

class FileManager {
    private:
        std::string base_path;
    public:
        FileManager();
        void set_base_path(std::string &path);
        int  create_directory(std::string &path);
        int  list_directory(std::string &path, std::string &out);
};