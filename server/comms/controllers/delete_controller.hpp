#pragma once
#include <string>
#include "../../../commons/file_manager/file.hpp"

class DeleteController {
    public:
        int delete_file(std::string filename, std::string username);
};