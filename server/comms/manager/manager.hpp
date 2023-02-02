#pragma once
#include <map>
#include <string>
#include "../../../types/user/user.hpp"

#define MAX_CONNECTIONS_PER_USER 2

class Manager {
    private:
        std::map<std::string, User> users;
    
    public:
        int add_user(std::string username);
        int remove_user(std::string username);
        int add_connection(std::string username, Socket connection);
        int remove_connection(std::string username, int sockfd);
        int broadcast(packet *p);
        int manage();
};