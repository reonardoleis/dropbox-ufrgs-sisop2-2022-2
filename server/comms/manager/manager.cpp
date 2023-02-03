#include "manager.hpp"
#include <pthread.h>

Manager::Manager() {
    this->server_file_manager = FileManager();
    std::string base_path = SYNC_DIRS_BASE_PATH;
    this->server_file_manager.set_base_path(base_path);
}

int Manager::add_user(std::string username)
{
    if (this->users.find(username) == this->users.end())
    {
        User user = User(username, MAX_CONNECTIONS_PER_USER);
        this->users.insert(std::pair<std::string, User>(username, user));
        return 0;
    }

    return -1;
}

int Manager::remove_user(std::string username)
{
    if (this->users.find(username) != this->users.end())
    {
        this->users.erase(username);
        return 0;
    }

    return -1;
}

int Manager::add_connection(std::string username, Socket connection)
{
    if (this->users.find(username) != this->users.end())
    {
        User user = this->users[username];
        return user.push_connection(connection);
    }

    return -1;
}

int Manager::remove_connection(std::string username, int sockfd)
{
    if (this->users.find(username) != this->users.end())
    {
        User user = this->users[username];
        return user.disconnect(sockfd);
    }

    return -1;
}

int Manager::broadcast(packet *p)
{
    for (auto user : this->users)
    {
        Socket *connections = user.second.get_connections();
        for (int i = 0; i < user.second.get_active_connections_count(); i++)
        {
            if (connections[i].sockfd > 0)
            {
                connections[i].write_packet(p);
            }
        }
    }
    return 0;
}

void* Manager::manage(void * manager)
{  
    Manager *m = (Manager *)manager;
    for (auto user : m->users)
    {
        Socket *connections = user.second.get_connections();
        for (int i = 0; i < user.second.get_active_connections_count(); i++)
        {
            if (connections[i].sockfd > 0)
            {
                pthread_t thread_id = 0;

                handle_connection_input *input = new handle_connection_input;
                input->connection = &connections[i];
                input->username = user.first;
                input->manager = m;

                m->lock.lock();
                pthread_create(&thread_id, NULL, Manager::handle_connection, (void *)input);
                m->active_connections_threads.push_back(thread_id);
                m->lock.unlock();
            }
        }
    }

    for (int i = 0; i < m->active_connections_threads.size(); i++)
    {
        pthread_join(m->active_connections_threads[i], NULL);
    }

    m->lock.unlock();
    
    return 0;
}

void * Manager::handle_connection(void *input)
{
    handle_connection_input *in = (handle_connection_input *)input;
    Socket *connection = in->connection;
    std::string username = in->username;

    packet p = connection->read_packet();
    cli_info info; 
    info.stamp(); 
    char message[3];
    std::sprintf(message, "%d", p.type);
    info.set("new packet received on Manager::handle_connection (" + std::string(message) + ")");
    info.print(std::cout);
    switch (p.type)
    {
    case packet_type::LOGOUT_REQ:
    {
        packet p = connection->build_packet(packet_type::LOGOUT_RESP, 0, 0, "successfully logged out");
        connection->write_packet(&p);
        connection->close_connection();

        printf("user %s has logged out\n", username.c_str());
    }
    case packet_type::SYNC_DIR_REQ:
    {
        cli_info info; cli_error error;
        SyncController sync_controller = SyncController(in->manager->server_file_manager);
        int err = sync_controller.sync_dir(username);

        uint16_t packet_type = 0;
        char *message = NULL;
        if (err < 0) {
            error.stamp();
            error.set("error syncing directory for user " + username);
            error.print(std::cout);
            packet_type = packet_type::SYNC_DIR_REFUSE_RESP;
            message = "error syncing directory";
        } else {
            info.stamp();
            info.set("successfully synced directory for user " + username);
            info.print(std::cout);
            packet_type = packet_type::SYNC_DIR_ACCEPT_RESP;
            message = "successfully synced directory";
        }

        packet p = connection->build_packet(packet_type, 0, 0, message);
        connection->write_packet(&p);
        break;
    }
    }
}
