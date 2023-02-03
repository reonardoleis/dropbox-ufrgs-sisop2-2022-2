#pragma once
#include <string>

class FileManager {
    private:
        std::string base_path;
    public:
        FileManager();
        void set_base_path(std::string &path);
        int create_directory(std::string &path);
};