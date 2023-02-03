#include "router.hpp"
#include <iostream>
#include "../manager/manager.hpp"
#include "../../../commons/packet.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include "../../../commons/ui/ui_template.hpp"

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

    cli_logger logger = cli_logger(frontend.get_log_stream());
    Manager manager;
    pthread_t manager_thread_id = 0;
    pthread_create(&manager_thread_id, NULL, Manager::manage, (void *)&manager);
    logger.set("Manager dispatched").stamp().info();
    

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

            manager.lock.lock();
            int is_new_user = manager.add_user(username);
            if (manager.add_connection(username, slave_socket) < 0)
            {
                logger.set("error while adding connection, max connections reached for user " + username).stamp().error();
                packet p = this->server_socket->build_packet(packet_type::LOGIN_REFUSE_RESP, 0, 0, "max connections reached");
                slave_socket.write_packet(&p);
                break;
            }
            manager.lock.unlock();

            if (is_new_user < 0)
                logger.set("new connection accepted for user " + username).stamp().info();
            else
                logger.set("new user added and new connection accepted for user " + username).stamp().warning();

            packet p = this->server_socket->build_packet(packet_type::LOGIN_ACCEPT_RESP, 0, 0, "connection accepted");
            slave_socket.write_packet(&p);
            break;
        }
        case packet_type::STOP_SERVER_REQ:
            routing = false;
            packet p = this->server_socket->build_packet(packet_type::STOP_SERVER_BROADCAST, 0, 0, "server stopped");
            slave_socket.write_packet(&p);
            manager.broadcast(&p);
            logger.set("Server stopped by network request").stamp().warning();
            break;
        }

        
    }
   
    pthread_join(manager_thread_id, NULL);

    return 0;
}