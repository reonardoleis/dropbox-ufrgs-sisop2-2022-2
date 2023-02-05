#include "./user.hpp"

User::User() {
    this->username = "";
    this->connections = new Socket[0];
    this->max_connections = 0;
}

User::User(std::string username, int max_connections)
{
    this->username = username;
    this->connections = new Socket[max_connections];
    for (int i = 0; i < max_connections; i++)
    {
        this->connections[i].sockfd = 0;
    }
    this->max_connections = max_connections;
}

std::string User::get_username()
{
    return this->username;
}

Socket *User::get_connections()
{
    return this->connections;
}

int User::push_connection(Socket connection)
{
    for (int i = 0; i < this->max_connections; i++)
    {
        if (this->connections[i].sockfd == 0)
        {
            this->connections[i] = connection;
            return 0;
        }
    }
    return -1;
}

int User::disconnect(int sockfd)
{
    for (int i = 0; i < this->max_connections; i++)
    {
        if (this->connections[i].sockfd == sockfd)
        {
            this->connections[i].close_connection();
            return 0;
        }
    }
    return -1;
}

int User::get_active_connections_count()
{
    int count = 0;
    for (int i = 0; i < this->max_connections; i++)
    {
        if (this->connections[i].sockfd != 0)
        {
            count++;
        }
    }
    return count;
}