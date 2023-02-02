#pragma once

#include "../socket/server_socket.hpp"

class Router {
    public:
        ServerSocket *server_socket;
        Router(ServerSocket *server_socket);
        // UploadController upload_controller;
        // DownloadController download_controller;
        // DeleteController delete_controller;
        // ListController list_controller;
        // SynchronizeController synchronize_controller;
        int start(); // start handling the messages
};


