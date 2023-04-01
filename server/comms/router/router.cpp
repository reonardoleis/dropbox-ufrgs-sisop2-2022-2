#include "router.hpp"
#include <iostream>
#include "../manager/manager.hpp"
#include "../../../commons/packet.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include <arpa/inet.h>
#include "../../../commons/ui/ui_template.hpp"

Router::Router(ServerSocket *server_socket)
{
    this->server_socket = server_socket;
}

int Router::start(std::vector<sockaddr_in> context, InternalRouter *p_internal_router)
{
    if (int err = this->server_socket->bind_and_listen() < 0)
    {
        return err;
    }

    bool routing = true;
    cli_logger logger = cli_logger(frontend.get_log_stream());
    Manager manager(&routing);
    manager.p_internal_router = p_internal_router;
    pthread_t manager_thread_id = 0;
    pthread_create(&manager_thread_id, NULL, Manager::manage, (void *)&manager);
    logger.set("Manager dispatched").stamp().info();

    pthread_t router_handle_connection_thread_id = 0;
    //TODO: Add backup list to struct declaration
    router_handle_connection_input *handled_connection = new router_handle_connection_input;
    handled_connection->server_socket = this->server_socket;
    handled_connection->is_router_routing = &routing;
    handled_connection->out_slave_socket = NULL;

    pthread_create(&router_handle_connection_thread_id, NULL, Router::handle_connection, (void *)handled_connection);
    Socket udp_sock;
    udp_sock.udp_client();
    //TODO: add ip:port in packet buffer
    packet p_hs;
    for(sockaddr_in addr : context)
    {
        // get ip and port from addr
        char ip[INET_ADDRSTRLEN];
        uint16_t port;
 
        inet_ntop(AF_INET, addr.sin_addr, ip, sizeof (ip));
        port = htons (sin.sin_port);

        char * port_str = std::to_string(port).c_str();
        char * ip_port =   strcat(ip, ":");
        ip_port = strcat(ip_port, port_str);
        p_hs =  = udp_sock.build_packet(packet_type::SERVER_HANDSHAKE, 0, 1, ip_port);
        udp_sock.udp_send(&p_hs, addr);
    }
    udp_sock.close_connection();
    while (routing)
    {
        bool is_master_socket_waiting = this->server_socket->get_is_waiting();
        if (!is_master_socket_waiting) {
            continue;
        }

        ServerSocket slave_socket = *handled_connection->out_slave_socket;
        this->server_socket->set_is_waiting(false);

        packet p = slave_socket.read_packet();

        logger.stamp().set("new packet on router").info();
        std::string message = "";
        switch (p.type)
        {
        case packet_type::LOGIN_REQ:
        {
            logger.stamp().set("Login request").info();
            std::string username = std::string(p._payload);

            manager.lock.lock();
            int is_new_user = manager.add_user(username);
            if (manager.add_connection(username, slave_socket) < 0)
            {

                logger.set("error while adding connection, max connections reached for user " + username).stamp().error();
                message = "Max connections reached";
                packet p = this->server_socket->build_packet(packet_type::LOGIN_REFUSE_RESP, 0, 0, message.c_str());
                slave_socket.write_packet(&p);
                break;
            }
            manager.lock.unlock();

            if (is_new_user < 0)
                logger.set("new connection accepted for user " + username).stamp().info();
            else
                logger.set("new user added and new connection accepted for user " + username).stamp().warning();

            message = "Connection accepted";
            packet p = this->server_socket->build_packet(packet_type::LOGIN_ACCEPT_RESP, 0, 0, message.c_str());
            slave_socket.write_packet(&p);
        }
        }
    }
    pthread_join(manager_thread_id, NULL);

    return 0;
}

void *Router::handle_connection(void *input)
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