#pragma once
#include "../socket/server_socket.hpp"
#include "../../../commons/file_manager/file_manager.hpp"
#include "../../../commons/user/user.hpp"
#include "../../../commons/packet.hpp"
#include "router.hpp"
#include ".././connections_manager/connections_manager.hpp"
#include <map>
#include <semaphore.h>
#include <vector>
#define TIMEOUTMS 5000 // 500ms

typedef struct _timeout_socket
{
    BackupClientSocket *sock;
    time_t *start;
    bool *timeout;
    sem_t *p_sm;
    
} timeout_socket_t;

/*
    Handles connection between servers, which includes:
        * State replication
        * Keepalive comunication
        * Voting algorithm
*/
class InternalRouter {
    private: 
        std::map<std::string, User> users;
        std::vector<server_ip_port_t> others;
        Router *router;
        bool is_master;
        int id;
    public:
        ServerSocket *server_socket;
        BackupClientSocket *client_socket;
        InternalRouter(ServerSocket *server_socket, ConnectionsManager *connections_manager);
        InternalRouter(ServerSocket *server_socket);
        InternalRouter(ServerSocket *server_socket, BackupClientSocket *client_socket);
        void start_vote();
        void broadcast(packet p);
        int broadcast_others();
        static void * start(void *input); // start handling the messages
        static void * handle_connection(void *input);
        static void * keepalive(void *input);
        static void * timeout(void *input);
        void set_is_master(bool is_master);
        bool get_is_master();
        void set_id(int id);
        int get_id();
        ConnectionsManager *connections_manager;
};