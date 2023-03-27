#pragma once

#include "../socket/server_socket.hpp"
#include "internal_router.hpp"
#include <vector>
class Router {
    public:
        ServerSocket *server_socket;
        Router(ServerSocket *server_socket);
        // UploadController upload_controller;
        // DownloadController download_controller;
        // DeleteController delete_controller;
        // ListController list_controller;
        // SynchronizeController synchronize_controller;
        int start(std::vector<sockaddr_in> context, InternalRouter *p_internal_router); // start handling the messages
        static void * handle_connection(void *input);
};

typedef struct router_handle_connection_input {
    ServerSocket *server_socket;
    bool *is_router_routing;
    ServerSocket *out_slave_socket;
    server_list_t backups;
} router_handle_connection_input;