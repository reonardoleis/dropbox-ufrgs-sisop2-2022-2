#pragma once
#include "../socket/server_socket.hpp"
#include "../../../commons/file_manager/file_manager.hpp"
#include "../../../commons/user/user.hpp"
#include "../../../commons/packet.hpp"
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
    public:
        InternalRouter();
        void start_vote();
        void broadcast(packet p);
        int start(); // start handling the messages
        static void * handle_connection(void *input);
}