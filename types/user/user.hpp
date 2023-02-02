#pragma once
#include <string>
#include "../socket/socket.hpp"
class User
{
private:
    std::string username;
    Socket *connections;
    int max_connections;
public:
    User();
    User(std::string username, int max_connections);
    std::string get_username();
    Socket *get_connections();
    int push_connection(Socket connection);
    int disconnect(int sockfd);
    int get_active_connections_count();
};