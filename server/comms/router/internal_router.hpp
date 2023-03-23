#pragma once
#include "../socket/server_socket.hpp"
#include "../../../commons/file_manager/file_manager.hpp"
#include "../../../commons/user/user.hpp"
#include "../../../commons/packet.hpp"
#include "router.hpp"
#include ".././connections_manager/connections_manager.hpp"
#include <map>

/*
    Handles connection between servers, which includes:
        * State replication
        * Keepalive comunication
        * Voting algorithm
*/
class InternalRouter {
    private: 
        std::map<std::string, User> users;
        server_list_t others;
        ServerSocket *server_socket;
        Router *router;
        bool is_master;
    public:
        InternalRouter(ServerSocket *server_socket, ConnectionsManager *connections_manager);
        InternalRouter(ServerSocket *server_socket);
        void start_vote();
        void broadcast(packet p);
        int broadcast_others();
        static void * start(void *input); // start handling the messages
        static void * handle_connection(void *input);
        void set_is_master(bool is_master);
        bool get_is_master();
        ConnectionsManager *connections_manager;
};