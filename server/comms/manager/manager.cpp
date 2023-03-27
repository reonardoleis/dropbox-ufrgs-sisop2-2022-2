#include "manager.hpp"
#include <pthread.h>

Manager::Manager()
{
    this->server_file_manager = FileManager();
    std::string base_path = SYNC_DIRS_BASE_PATH;
    this->server_file_manager.set_base_path(base_path);
    this->is_router_routing = new bool(true);
}

Manager::Manager(bool *is_router_routing)
{
    this->server_file_manager = FileManager();
    std::string base_path = SYNC_DIRS_BASE_PATH;
    this->server_file_manager.set_base_path(base_path);
    this->is_router_routing = is_router_routing;
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

void *Manager::manage(void *manager)
{
    Manager *m = (Manager *)manager;
    cli_logger logger = cli_logger(frontend.get_log_stream());
    while (*(m->is_router_routing))
    {

        for (auto user : m->users)
        {

            Socket *connections = user.second.get_connections();

            for (int i = 0; i < user.second.get_active_connections_count(); i++)
            {
                if (connections[i].sockfd > 0)
                {
                    if (!connections[i].get_is_waiting())
                    {

                        pthread_t thread_id = 0;

                        handle_connection_input *input = new handle_connection_input;
                        input->connection = &connections[i];
                        input->username = user.first;
                        input->manager = m;
                        m->lock.lock();
                        connections[i].set_is_waiting(true);
                        pthread_create(&thread_id, NULL, Manager::handle_connection, (void *)input);
                        m->active_connections_threads.push_back(thread_id);
                        m->lock.unlock();
                    }
                }
            }
        }
    }

    for (int i = 0; i < m->active_connections_threads.size(); i++)
    {
        pthread_join(m->active_connections_threads[i], NULL);
    }

    return NULL;
}

void *Manager::handle_connection(void *input)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    logger.set("trying to handle a connection...").stamp().info();

    handle_connection_input *in = (handle_connection_input *)input;
    Socket *connection = in->connection;
    std::string username = in->username;

    packet p = connection->read_packet();
    in->manager->p_internal_router->broadcast_others(p, in->username);
    logger.set("new packet received on Manager::handle_connection (" + std::to_string(p.type) + ")").stamp().info();
    switch (p.type)
    {
    case packet_type::LOGOUT_REQ:
    {
        packet p = connection->build_packet(packet_type::LOGOUT_RESP, 0, 0, "successfully logged out");
        connection->write_packet(&p);
        connection->close_connection();

        logger.set("user " + username + " has logged out").stamp().info();

        break;
    }
    case packet_type::SYNC_DIR_REQ:
    {

        SyncController sync_controller = SyncController(in->manager->server_file_manager);
        int err = sync_controller.sync_dir(username);

        uint16_t p_type = 0;
        std::string message = "";
        if (err < 0)
        {
            logger.stamp().set("error syncing directory for user " + username).error();
            p_type = packet_type::SYNC_DIR_REFUSE_RESP;
            message = "error syncing directory";
            packet p = connection->build_packet(p_type, 0, 0, message.c_str());
            connection->write_packet(&p);
        }
        else
        {
            if(err == 1)
            {
                logger.stamp().set("sync_directory for user " + username + " already exists").info();
                in->manager->sync_files(username, connection);
            }
            else
            {
                logger.stamp().set("successfully created directory for user " + username).info();
                p_type = packet_type::SYNC_DIR_ACCEPT_RESP;
                message = "successfully synced directory";
                packet p = connection->build_packet(p_type, 0, 0, message.c_str());
                connection->write_packet(&p);
            }
        }

        break;
    }
    case packet_type::STOP_SERVER_REQ:
    {
        logger.set("Server stopped by network request").stamp().warning();
        packet p = connection->build_packet(packet_type::STOP_SERVER_BROADCAST, 0, 0, "server stopped");
        //connection->write_packet(&p);
        in->manager->broadcast(&p);

        usleep(100);
        in->manager->close_all_connections();

        break;
    }
    case packet_type::UPLOAD_REQ:
    {
        UploadController upload_controller = UploadController();
        cli_logger logger = cli_logger(frontend.get_log_stream());
        serialized_file_t serialized_file = File::from_data(p._payload);
        logger.set("uploading file " + std::string(serialized_file.filename) + " for user " + username).stamp().info();

        File file = File("");
        file.deserialize(serialized_file);
        logger.set("file " + file.filename + " received").stamp().info();
        int err = upload_controller.upload(file, username);
        std::string message = "";
        int p_type = 0;
        if (err < 0)
        {
            logger.set("error uploading file for user " + username).stamp().error();
            p_type = packet_type::UPLOAD_REFUSE_RESP;
            message = "error uploading file";
        }
        else
        {
            logger.set("successfully uploaded file for user " + username).stamp().info();
            p_type = packet_type::UPLOAD_ACCEPT_RESP;
            message = "successfully uploaded file";
        }

        packet p = connection->build_packet(p_type, 0, 0, message.c_str());
        connection->write_packet(&p);

        // broadcast the upload to other user connections
        char * file_bytes = file.to_data();
        uint32_t size = file.get_payload_size();
        packet broadcast_packet = connection->build_packet_sized(packet_type::UPLOAD_BROADCAST, 0, 0, size, file_bytes);
        err = in->manager->broadcast_to_user(username, -1, &broadcast_packet);
        logger.set("Broadcasting for " + username).stamp().info();
        break;
    }
    case packet_type::DOWNLOAD_REQ:
    {
        DownloadController download_controller = DownloadController();
        cli_logger logger = cli_logger(frontend.get_log_stream());

        char *filename = p._payload;
        std::string filename_str = filename;
        logger.set("downloading " + filename_str + " for user " + username).stamp().info();

        File *file;
        int err = download_controller.download(&file, filename, username);
        char* message = "";
        int p_type = 0;
        uint32_t size = 0;

        if (err < 0)
        {
            logger.set("error downloading file for user " + username).stamp().error();
            p_type = packet_type::DOWNLOAD_REFUSE_RESP;
            message = "Error downloading file";
            size = strlen(message);
        }
        else
        {
            logger.set("successfully downloaded file for user " + username).stamp().info();

            p_type = packet_type::DOWNLOAD_ACCEPT_RESP;

            message = file->to_data();
            size = file->get_payload_size();

            logger.set("file content: " + std::string(message + sizeof(int) + 256)).stamp().info();
            logger.set("file name: " + std::string(message + sizeof(int))).stamp().info();
            logger.set("size: " + std::to_string(size)).stamp().info();
            logger.set("sizeof(payload): " + std::to_string(sizeof(message))).stamp().info();
            logger.set("sizeof(serialized_file_t): " + std::to_string(sizeof(serialized_file_t))).stamp().info();
        }
        packet p = connection->build_packet_sized(p_type, 0, 0, size, message);
        logger.set("p.payload: " + std::string(p._payload + sizeof(int) +256)).stamp().info();

        connection->write_packet(&p);
        
        break;
    }
    case packet_type::LIST_REQ:
    {
        SyncController sync_controller = SyncController(in->manager->server_file_manager);
        cli_logger logger = cli_logger(frontend.get_log_stream());
        logger.set("listing files for user " + username).stamp().info();
        char cCurrentPath[FILENAME_MAX];
        if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
        {
            logger.set("Failed to get running directory: ERRNO " + std::to_string(errno)).stamp().error();
            return NULL;
        }
        std::string base = std::string(cCurrentPath);
        std::string path = std::string(base + "/sync_directories/sync_dir_" + username);
        std::string files = "";
        int err = in->manager->server_file_manager.list_directory(path, files);
        logger.set("listing files for user " + username).stamp().info();
        usleep(100);
        std::string message = "";
        int p_type = 0;
        int size = 0;

        if (err < 0)
        {
            logger.set("error listing files for user " + username).stamp().error();
            p_type = packet_type::LIST_REFUSE_RESP;
            message = "error listing files";
            size = message.length();
        }
        else
        {
            logger.set("successfully listed files for user " + username).stamp().info();
            p_type = packet_type::LIST_ACCEPT_RESP;
            message = files;
            size = files.length();
            logger.set("successfully listed files for user " + message).stamp().info();
        }

        packet p = connection->build_packet_sized(p_type, 0, 0, size + 1, message.c_str());
        connection->write_packet(&p);
        break;
    }
    case packet_type::DELETE_REQ:
    {
        DeleteController delete_controller = DeleteController();
        cli_logger logger = cli_logger(frontend.get_log_stream());
        std::string filename = p._payload;
        logger.set("Deleting file " + filename + " for user " + username).stamp().info();
        int err = delete_controller.delete_file(filename, username);
        
        std::string message = "";
        int p_type = 0;
        int size = 0;

        if (err < 0)
        {
            logger.set("error deleting file " + filename + " for user " + username).stamp().error();
            p_type = packet_type::DELETE_REFUSE_RESP;
            message = "error deleting file";
            size = message.length();
        }
        else
        {
            logger.set("successfully deleted " + filename + " for user " + username).stamp().info();
            p_type = packet_type::DELETE_ACCEPT_RESP;
            message = filename;
            size = message.length();
        }

        packet p = connection->build_packet_sized(p_type, 0, 0, size + 1, message.c_str());
        connection->write_packet(&p);

        // broadcast the upload to other user connections
        packet broadcast_packet = connection->build_packet_sized(packet_type::DELETE_BROADCAST, 0, 0, size + 1, message.c_str());
        err = in->manager->broadcast_to_user(username, connection->sockfd, &broadcast_packet);

        break;
    }
    case REDUNDANCY_REQ:
    {
        
    }
    default:
    {
        packet p = connection->build_packet(packet_type::UNKNOWN_RESP, 0, 0, "Unrecognized packet type");
        connection->write_packet(&p);
    }
    }

    connection->set_is_waiting(false);
    return NULL;
}

int Manager::broadcast_to_user(std::string username, int except_socketfd, packet *p)
{
    if (this->users.find(username) == this->users.end())
    {
        return -1;
    }
    cli_logger logger = cli_logger(frontend.get_log_stream());
    User user = this->users[username];
    Socket *connections = user.get_connections();
    int active_connections_count = user.get_active_connections_count();
    //logger.set(std::string("Closing ") + std::to_string(active_connections_count) + std::string(" connections for user") + username).stamp().warning();

    for (int i = 0; i < active_connections_count; i++)
    {
        if (connections[i].sockfd != except_socketfd)
        {
            connections[i].write_packet(p);
        }
    }

    return 0;
}

void Manager::sync_files(std::string username, Socket *connection)
{
    DownloadController download_controller = DownloadController();
    cli_logger logger = cli_logger(frontend.get_log_stream());
    std::string out, filename, _meta;
    char cCurrentPath[FILENAME_MAX];
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        logger.set("Failed to get running directory: ERRNO " + std::to_string(errno)).stamp().error();
        exit(-1);
    }
    std::string b = std::string(cCurrentPath);
    std::string user_path = std::string(b + "/sync_directories/sync_dir_"+username+"/");
    //std::string user_path = "/sync_dir_" + username;
    int total_files = this->server_file_manager.list_directory(user_path, out);
    
    //printf("OUT: %s", out.c_str());
    std::stringstream sfs; sfs << out;
    if(total_files > 0)
    {
        int curr_file = 1;
        std::getline(sfs, _meta);
        while(sfs.rdbuf()->in_avail() > 0)
        {
            std::getline(sfs, filename);
            for(int i = 0; i < 4; i++){std::getline(sfs, _meta);}
            File *file;
            int err = download_controller.download(&file, filename, username);
            uint32_t size = file->get_payload_size();
            char *payload = file->to_data();
            logger.set(file->filename).stamp().warning();
            packet p = connection->build_packet_sized(packet_type::SYNC_DIR_ACCEPT_RESP, curr_file, total_files, size, payload);
            curr_file += 1;
            connection->write_packet(&p);
        }
    }
    else
    {
        logger.set("No files to sync").stamp().warning();
        packet p = connection->build_packet_sized(packet_type::SYNC_DIR_ACCEPT_RESP, 0, 0, 1, "");
        connection->write_packet(&p);
    }
}

void Manager::close_all_connections()
{
    for (auto user : this->users)
    {
        Socket *connections = user.second.get_connections();
        for (int i = 0; i < user.second.get_active_connections_count(); i++)
        {
            if (connections[i].sockfd > 0)
            {
                connections[i].close_connection();
            }
        }
    }

    *(this->is_router_routing) = false;
}
