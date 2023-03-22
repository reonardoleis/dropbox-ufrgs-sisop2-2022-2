#include "internal_router.hpp"

InternalRouter::InternalRouter(ServerSocket *server_socket)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
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
    InternalRouter *self = (InternalRouter *)input;
    bool routing = true;
    cli_logger logger = cli_logger(frontend.get_log_stream());

    logger.stamp().set("Starting internal router...").info();

    pthread_t router_handle_connection_thread_id = 0;
    // TODO: Add backup list to struct declaration
    router_handle_connection_input *handled_connection = new router_handle_connection_input;
    handled_connection->server_socket = self->server_socket;
    handled_connection->is_router_routing = &routing;
    handled_connection->out_slave_socket = NULL;

    pthread_create(&router_handle_connection_thread_id, NULL, Router::handle_connection, (void *)handled_connection);
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
