#pragma once
#include <map>
#include <string>
#include "../../../commons/user/user.hpp"
#include <vector>
#include <mutex>

#define MAX_CONNECTIONS_PER_USER 2

typedef struct handle_connection_input {
    Socket *connection;
    std::string username;
} handle_connection_input;

class Manager {
    private:
        std::map<std::string, User> users;
        std::vector<pthread_t> active_connections_threads;

    public:
        int add_user(std::string username);
        int remove_user(std::string username);
        int add_connection(std::string username, Socket connection);
        int remove_connection(std::string username, int sockfd);
        int broadcast(packet *p);
        static void * manage(void *manager);
        static void * handle_connection(void *input);
        std::mutex lock;
};