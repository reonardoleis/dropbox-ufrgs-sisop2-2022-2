#pragma once
#include "../../../commons/file_manager/file.hpp"
#include "../../../commons/user/user.hpp"
class UploadController {
    public:
        int upload(File file, std::string username);
};