#include "download_controller.hpp"
#include "../../../commons/ui/cli_types.hpp"

int DownloadController::download(File **out, std::string filename, std::string username) {
   
    *out = new File(filename);
    std::string path = "./sync_directories/sync_dir_" + username;
    if ((*out)->read_file(path) < 0) {
        return -1;
    }


    return 0;
}