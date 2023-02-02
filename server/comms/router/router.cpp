#include "router.hpp"
#include <iostream>
#include "../manager/manager.hpp"
#include "../../../types/packet.hpp"

Router::Router(ServerSocket *server_socket)
{
    this->server_socket = server_socket;
}

int Router::start()
{
    if (int err = this->server_socket->bind_and_listen() < 0)
    {
        return err;
    }

    Manager manager;
    bool routing = true;
    while (routing)
    {
        ServerSocket slave_socket = this->server_socket->accept_connection();
        packet p = slave_socket.read_packet();
        switch (p.type)
        {
        case packet_type::LOGIN_REQ:
        {
            std::string username = std::string(p._payload);
            int is_new_user = manager.add_user(username);
            if (manager.add_connection(username, slave_socket) < 0)
            {
                printf("error while adding connection, max connections reached (user %s)", username.c_str());
                packet p = this->server_socket->build_packet(packet_type::LOGIN_REFUSE_RESP, 0, 0, "max connections reached");
                slave_socket.write_packet(&p);
                break;
            }

            if (is_new_user < 0)
                printf("new connection accepted for user %s", username.c_str());
            else
                printf("new user added and new connection accepted for user %s", username.c_str());

            packet p = this->server_socket->build_packet(packet_type::LOGIN_ACCEPT_RESP, 0, 0, "connection accepted");
            slave_socket.write_packet(&p);
            break;
        }
        case packet_type::STOP_SERVER_REQ:
            routing = false;
            packet p = this->server_socket->build_packet(packet_type::STOP_SERVER_BROADCAST, 0, 0, "server stopped");
            slave_socket.write_packet(&p);
            manager.broadcast(&p);
            printf("server stopped");
            break;
        }

        manager.manage();
    }
}