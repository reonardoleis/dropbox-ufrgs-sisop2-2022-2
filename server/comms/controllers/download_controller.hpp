#pragma once
#include <string>
#include "../../../commons/file_manager/file.hpp"

class DownloadController {
    public:
        int download(File ** out, std::string filename, std::string username);
};