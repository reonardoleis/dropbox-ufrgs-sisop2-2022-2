#include "connections_manager.hpp"
#include "../router/internal_router.hpp"

ConnectionsManager::ConnectionsManager(BackupClientSocket *backup_client_socket)
{
    this->backup_client_socket = backup_client_socket;
}

void * ConnectionsManager::keep_alive(void * input)
{
   InternalRouter * internal_router = (InternalRouter *) input;
   cli_logger logger = cli_logger(frontend.get_log_stream());
   while (!internal_router->get_is_master()) {
    logger.set("keep alive...").stamp().info();
    packet p = internal_router->connections_manager->backup_client_socket->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "a");
    internal_router->connections_manager->backup_client_socket->write_packet(&p);
    sleep(1);
 
   }
    logger.set("keep alive exiting...").stamp().info();
}

void ConnectionsManager::set_backup_client_socket(BackupClientSocket *backup_client_socket)
{
    this->backup_client_socket = backup_client_socket;
}

BackupClientSocket *ConnectionsManager::get_backup_client_socket()
{
    return this->backup_client_socket;
}