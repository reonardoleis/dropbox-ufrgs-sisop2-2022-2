#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#include "./comms/router/internal_router.hpp"
#include "../commons/ui/cli_types.hpp"
#include "../commons/ui/ui_template.hpp"
#include "./comms/socket/backup_client_socket.hpp"
#include "./comms/connections_manager/connections_manager.hpp"
#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    // TODO: Recieve master on startup and ask(?) for other backups (handshake) SERVER_HANDSHAKE and SERVER_KEEPALIVE
    cli_logger logger = cli_logger(frontend.get_log_stream());
    int port = 8080;
    char *role = "";
    std::string master_ip = "";
    int master_port = 0;

    if (argc < 3)
    {
        printf("Usage: ./server <port> <master|backup> [master_ip] [master_port]");
        return 1;
    }

    if (argc >= 2)
    {
        port = atoi(argv[1]);
        role = argv[2];
        if (strcmp(role, "master") != 0 && strcmp(role, "backup") != 0)
        {
            printf("Usage: ./server <port> <master|backup> [master_ip] [master_port]");
            return 1;
        }
    }

    if (argc > 3)
    {
        if (argc != 5)
        {
            printf("Usage: ./server <port> <master|backup> <master_ip> <master_port>");
            return 1;
        }

        master_ip = argv[3];
        master_port = atoi(argv[4]);
    }

    // print all the arguments (port, role, etc)
    logger.set("Port: " + to_string(port)).stamp().info();
    logger.set("Role: " + string(role)).stamp().info();
    logger.set("Master IP: " + string(master_ip)).stamp().info();
    logger.set("Master Port: " + to_string(master_port)).stamp().info();

    pthread_t ui_thread_id = 0;
    pthread_create(&ui_thread_id, NULL, ui_template::thread_ready, &frontend);
    logger.set("Starting server on port: " + to_string(port)).stamp().info();

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);
    ServerSocket internal_socket = ServerSocket(port + 50, MASTER_SOCKET_QUEUE_SIZE);
    if (int err = internal_socket.bind_and_listen() < 0)
    {
        return err;
    }

    Router router = Router(&master_socket);
    InternalRouter *internal_router = NULL;
    BackupClientSocket *client_soc;
    pthread_t internal_router_thread_id = 0;
    std::vector<sockaddr_in> *p_context = new std::vector<sockaddr_in>;

    if(strcmp(role, "backup") == 0)
    {
        client_soc = new BackupClientSocket(master_ip.c_str(), master_port+50);
    }
    // RESTARTING BREAKPOINT 
    while(strcmp(role, "backup") == 0)
    {
        logger.set("Connecting to master...").stamp().info();
        try{
            int err = client_soc->connect_to_server();
            if (err != 0)
            {
                logger.set("Failed to connect to master").stamp().error();
                return 1;
            }
        }
        catch(SocketError err)
        {
            logger.set("Failed to connect to server with error [" + std::string(strerror(err)) + "]").stamp().error();
            usleep(200);
            exit(errno);
        }
        logger.set("Connected to master...").stamp().info();
        packet *p;
        packet p1 = client_soc->build_packet(packet_type::JOIN_REQ, 0, 1, std::to_string(port).c_str());
        p = &p1;
        client_soc->write_packet(p);
        
        //ConnectionsManager connections_manager = ConnectionsManager(client_soc);
        internal_router = new InternalRouter(&internal_socket, client_soc);
        internal_router->set_is_master(false);
        packet p2 = client_soc->read_packet();
        p = &p2;
        if(p->type == packet_type::JOIN_RESP)
        {
            char * ip_port = p->_payload;
            std::string ip_port_str = std::string(ip_port);
            logger.stamp().set(std::string("Received join response ") + ip_port_str).info();
            std::string ip = ip_port_str.substr(0, ip_port_str.find(":"));
            std::string port_str = ip_port_str.substr(ip_port_str.find(":") + 1, ip_port_str.length());
            int port = std::stoi(port_str);
            server_ip_port_t adjacent;
            adjacent.server_ip = ip;
            adjacent.server_port = port;
            internal_router->set_adjacent(adjacent);
            packet p3 = client_soc->read_packet();
            p = new packet;
            *p = p3;
            p->_payload = (char *)malloc(p3.length);
            memcpy(p->_payload, p3._payload, p3.length);
        }
        if(p->type == packet_type::YOUR_ID_RESP)
        {
            logger.stamp().set("Received your id response").info();
            std::string ip = p->_payload;
            int id_int = p->total_size;
            internal_router->set_id(id_int);
            internal_router->set_ip(ip.c_str());
        }
        internal_router_thread_id = 0;
        delete p_context;
        p_context = (std::vector<sockaddr_in> *)InternalRouter::start((void *)internal_router);

        if(!internal_router->should_relaunch())
        {
            break;
        }

    } 
    if(internal_router == NULL)
    {
        internal_router = new InternalRouter(&internal_socket);
        internal_router->set_is_master(true);
        internal_router_thread_id = 0;
        pthread_create(&internal_router_thread_id, NULL, InternalRouter::start, (void *)internal_router);
    }
    else
    {
        internal_router = new InternalRouter(&internal_socket);
        internal_router->set_is_master(true);
        internal_router_thread_id = 0;
        pthread_create(&internal_router_thread_id, NULL, InternalRouter::start, (void *)internal_router);
    }
    

    router.start(p_context, internal_router);

    logger.set("Closing server...").stamp().info();
    frontend.stop_ui();
    pthread_join(ui_thread_id, NULL);
    pthread_join(internal_router_thread_id, NULL);
    master_socket.close_connection();
    return 0;
}