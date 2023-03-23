#pragma once
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include "../controllers/sync_controller.hpp"
#include "../controllers/upload_controller.hpp"
#include "../controllers/download_controller.hpp"
#include "../controllers/delete_controller.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include "../../../commons/ui/ui_template.hpp"
#include "../../../commons/packet.hpp"

#define MAX_CONNECTIONS_PER_USER 2
#define SYNC_DIRS_BASE_PATH "./sync_directories"

typedef struct backup_t {
    Socket * connection;
    std::string ip_port;
} backup_t;

class InternalManager {
    private:
        std::vector<backup_t> backups;
        std::vector<pthread_t> active_connections_threads;

    public:
        int add_backup(backup_t backup);
        int broadcast(packet *p);
        static void * manage(void *manager);
        static void * handle_connection(void *input);
        std::mutex lock;
        Manager();
        Manager(bool *is_router_routing);
        bool *is_router_routing;
};

typedef struct handle_connection_input {
    Socket *connection;
    std::string username;
    Manager *manager;
} handle_connection_input;
