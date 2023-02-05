#include "download_controller.hpp"
#include "../../../commons/ui/cli_types.hpp"

int DownloadController::download(File **out, std::string filename, std::string username) {
   
    *out = new File(filename);
    std::string path = "./sync_directories/sync_dir_" + username;
    if ((*out)->read_file(path) < 0) {
        return -1;
    }

    cli_logger logger = cli_logger(frontend.get_log_stream());
    std::string foo = "payload " + std::string((*out)->data) +  "\n" + "size " 
    + std::to_string((*out)->file_size) + "\n" + "path " + path + "\n" + "filename " + filename + "\n" + "username " + username;

    logger.set(foo).stamp().info();

    return 0;
}