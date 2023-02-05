#pragma once
#include <map>
#include <string>
#include "../../../commons/user/user.hpp"
#include <vector>
#include <mutex>
#include "../../../commons/file_manager/file_manager.hpp"
#include "../controllers/sync_controller.hpp"
#include "../controllers/upload_controller.hpp"
#include "../controllers/download_controller.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include "../../../commons/ui/ui_template.hpp"


#define MAX_CONNECTIONS_PER_USER 2
#define SYNC_DIRS_BASE_PATH "./sync_directories"

class Manager {
    private:
        std::map<std::string, User> users;
        std::vector<pthread_t> active_connections_threads;
        FileManager server_file_manager;

    public:
        int add_user(std::string username);
        int remove_user(std::string username);
        int add_connection(std::string username, Socket connection);
        int remove_connection(std::string username, int sockfd);
        int broadcast(packet *p);
        static void * manage(void *manager);
        static void * handle_connection(void *input);
        pthread_mutex_t lock;
        Manager();
        Manager(bool *is_router_routing);
        void close_all_connections();
        bool *is_router_routing;
};

typedef struct handle_connection_input {
    Socket *connection;
    std::string username;
    Manager *manager;
} handle_connection_input;
