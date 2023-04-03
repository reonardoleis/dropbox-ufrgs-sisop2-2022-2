#pragma once
#include "../socket/server_socket.hpp"
#include "../../../commons/file_manager/file_manager.hpp"
#include "../../../commons/user/user.hpp"
#include "../../../commons/packet.hpp"
#include "../controllers/upload_controller.hpp"
#include "../controllers/delete_controller.hpp"
//#include "router.hpp"
#include ".././connections_manager/connections_manager.hpp"
#include <map>
#include <semaphore.h>
#include <vector>
#define TIMEOUTMS 100000 // 0.1s
#define SYNC_DIRS_BASE_PATH "./sync_directories"

typedef struct _timeout_socket
{
    BackupClientSocket *sock;
    time_t *start;
    bool *timeout;
    sem_t *p_sm;
    
} timeout_socket_t;

typedef struct _serialized_id_ipport
{
    int id;
    int port;
    char *ip;
} serialized_id_ipport_t;

typedef struct _signed_payload
{
    packet p;
    char *username;
} signed_payload_t;


/*
    Handles connection between servers, which includes:
        * State replication
        * Keepalive comunication
        * Voting algorithm
*/
class InternalRouter {
    private: 
        std::vector<server_ip_port_t> others;
        bool is_master;
        bool relaunch;
        int id;
        int _id;
        server_ip_port_t adjacent;
        std::string external_ip;
        int next_backup_id;
    public:
        std::map<std::string, std::vector<sockaddr_in>> users;
        ServerSocket *server_socket;
        BackupClientSocket *client_socket;
        InternalRouter(ServerSocket *server_socket);
        InternalRouter(ServerSocket *server_socket, BackupClientSocket *client_socket);
        void start_vote();
        void broadcast(packet p);
        int broadcast_others(packet p, std::string user);
        signed_payload_t extract_signature(packet p);
        packet sign_packet(packet p, std::string user);
        static void * start(void *input); // start handling the messages
        static void * handle_connection(void *input);
        static void * keepalive(void *input);
        static void * timeout(void *input);
        void set_is_master(bool is_master);
        bool get_is_master();
        int get_next_backup_id();
        void set_id(int id);
        int get_id();
        void set_ip(const char *ip);
        std::string get_ip();
        void set_adjacent(server_ip_port_t adjacent);
        server_ip_port_t get_adjacent();
        void set_relaunch();
        bool should_relaunch();
};