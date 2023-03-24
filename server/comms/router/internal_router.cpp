#include "internal_router.hpp"

InternalRouter::InternalRouter(ServerSocket *server_socket)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
}

InternalRouter::InternalRouter(ServerSocket *server_socket, ConnectionsManager *connections_manager)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
    this->connections_manager = connections_manager;
}

InternalRouter::InternalRouter(ServerSocket *server_socket, BackupClientSocket *client_socket)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
    this->client_socket = client_socket;
}

void InternalRouter::start_vote()
{
}

void InternalRouter::broadcast(packet p)
{
}

int InternalRouter::broadcast_others()
{
    return 0;
}

void * InternalRouter::start(void *input)
{
    InternalRouter *self = (InternalRouter *) input;
    cli_logger logger = cli_logger(frontend.get_log_stream());
    if (int err = self->server_socket->bind_and_listen() < 0)
    {
        return NULL;
    }
    bool timeout = false;
    time_t start = 0;
    timeout_socket_t ts {self->client_socket, &start, &timeout};
    if (!self->get_is_master()) {
        logger.stamp().set("starting connections manager because I'm not the master").info();
        pthread_t keep_alive_thread_id = 0;
        pthread_t timeout_thread_id = 0;
        pthread_create(&keep_alive_thread_id, NULL, InternalRouter::keepalive, (void *)&ts);
        *(ts.start) = time(NULL);
        pthread_create(&keep_alive_thread_id, NULL, InternalRouter::timeout, (void *)&ts);
    }

    bool routing = true;
   

    logger.stamp().set("Starting internal router...").info();

    pthread_t router_handle_connection_thread_id = 0;
    // TODO: Add backup list to struct declaration
    router_handle_connection_input *handled_connection = new router_handle_connection_input;
    handled_connection->server_socket = self->server_socket;
    handled_connection->is_router_routing = &routing;
    handled_connection->out_slave_socket = NULL;

    pthread_create(&router_handle_connection_thread_id, NULL, Router::handle_connection, (void *)handled_connection);
    //TODO: Complete internal_manager (ONLY NEEDED FOR MASTER)
    while (routing)
    {
        bool is_master_socket_waiting = self->server_socket->get_is_waiting();
        if (!is_master_socket_waiting)
        {
            continue;
        }

        ServerSocket backup_slave_socket = *handled_connection->out_slave_socket;
        self->server_socket->set_is_waiting(false);

        packet p = backup_slave_socket.read_packet();

        logger.stamp().set("new packet on router").info();
        std::string message = "";
        switch (p.type)
        {
        case packet_type::JOIN_REQ:
        {
            logger.stamp().set("Server join request").info();
            std::string ip_port = std::string(p._payload);
            logger.stamp().set("IP PORT: " + ip_port).info();
            message = "Server join accepted";
            packet p = self->server_socket->build_packet(packet_type::JOIN_RESP, 0, 0, message.c_str());
            backup_slave_socket.write_packet(&p);
            break;
        }
        case packet_type::SERVER_KEEPALIVE:
        {
            logger.stamp().set("Keep alive received").info();
            break;
        }
        }
    }

    return 0;
}

void *InternalRouter::handle_connection(void *input)
{
    router_handle_connection_input *in = (router_handle_connection_input *)input;
    ServerSocket *server_socket = in->server_socket;
    bool *is_routing = in->is_router_routing;
    while (*is_routing)
    {
        if (!server_socket->get_is_waiting())
        {
            ServerSocket out_slave_socket = server_socket->accept_connection();
            in->out_slave_socket = new ServerSocket(out_slave_socket);
            server_socket->set_is_waiting(true);
        }
    }

    return NULL;
}

void * InternalRouter::keepalive(void * input)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    timeout_socket_t *ts = (timeout_socket_t *)input;
    BackupClientSocket *sock  = ts->sock;
    time_t *start = ts->start;
    packet p = sock->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
    usleep(100);
    while (!(*(ts->timeout)))
    {
        sock->write_packet(&p);
        printf("\n\nkeepalive1");
        packet r = sock->read_packet();
        if(r.type == packet_type::SERVER_KEEPALIVE)
        {
            *start = time(NULL);
            printf("\nalive");
        }
        printf("\nkeepalive2\n\n");
        usleep(100);
    }
    logger.set("keepalive").stamp().error();
    return NULL;
    
    /*
   InternalRouter * internal_router = (InternalRouter *) input;
   cli_logger logger = cli_logger(frontend.get_log_stream());
   while (!internal_router->get_is_master()) {
    logger.set("keep alive...").stamp().info();
    packet p = internal_router->connections_manager->backup_client_socket->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "a");
    internal_router->connections_manager->backup_client_socket->write_packet(&p);
    sleep(1);
 
   }
    logger.set("keep alive exiting...").stamp().info();
    */
}

void * InternalRouter::timeout(void * input)
{
    timeout_socket_t *ts = (timeout_socket_t *)input;
    bool *timeout = ts->timeout;
    time_t *start = ts->start;
    BackupClientSocket *sock = ts->sock;
    packet p = sock->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
    sock->write_packet(&p);
    while ((time(NULL) - *start)*1000 < TIMEOUTMS)
    {
        usleep(10);
    }
    *timeout = true;
    return NULL;
}


void InternalRouter::set_is_master(bool is_master)
{
    this->is_master = is_master;
}

bool InternalRouter::get_is_master()
{
    return this->is_master;
}