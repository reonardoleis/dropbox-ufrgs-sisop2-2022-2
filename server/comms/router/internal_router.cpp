#include "internal_router.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

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
    bool timeout = false;
    time_t start = 0;
    pthread_t keep_alive_thread_id = 0;
    pthread_t timeout_thread_id = 0;
    sem_t sm;
    timeout_socket_t ts {self->client_socket, &start, &timeout, &sm};
    std::vector<sockaddr_in> *p_context = new std::vector<sockaddr_in>;
    if (!self->get_is_master()) {
        ServerSocket slave_socket = self->server_socket->accept_connection();

        logger.stamp().set("starting connections manager because I'm not the master").info();
        pthread_create(&keep_alive_thread_id, NULL, InternalRouter::keepalive, (void *)&ts);
        *(ts.start) = time(NULL);
        //pthread_create(&timeout_thread_id, NULL, InternalRouter::timeout, (void *)&ts);
        //usleep(100);
        //pthread_join(timeout_thread_id, NULL);
        pthread_join(keep_alive_thread_id, NULL);
        logger.set("Starting voting round").stamp().warning();
        // Voting round to determine new master;
        while (!self->get_is_master()) {
            packet p = self->client_socket->read_packet();
            switch (p.type) {
                case packet_type::JOIN_RESP:
                {
                    logger.stamp().set("Received join response").info();
                    char * ip_port = p._payload;
                    std::string ip_port_str = std::string(ip_port);
                    std::string ip = ip_port_str.substr(0, ip_port_str.find(":"));
                    std::string port_str = ip_port_str.substr(ip_port_str.find(":") + 1, ip_port_str.length());
                    int port = std::stoi(port_str);

                    _server_ip_port *ip_port = new _server_ip_port;
                    ip_port->server_ip = ip;
                    ip_port->server_port = port;
                    ip_port->socket = slave_socket;

                    self->others.push_back(*ip_port);

                    break;
                }
                case packet_type::YOUR_ID_RESP:
                {
                    logger.stamp().set("Received your id response").info();
                    char * id = p._payload;
                    int id_int = std::stoi(std::string(id));
                    self->set_id(id_int);
                    break;
                }
                case packet_type::VOTE:
                {
                    logger.stamp().set("Received vote").info();
                    char * id = p._payload;
                    int id_int = std::stoi(std::string(id));
                    if (self->get_id() == id_int) {
                        logger.stamp().set("I'm the master >:").info();
                        self->set_is_master(true);
                    } else if (self->get_id() < id_int) {
                        logger.stamp().set("I want to be the master").info();
                        
                    } else {
                        logger.stamp().set("I don't want to be the master").info();
                    }
                    break;
                }
            }
        }
    }
    else
    {
        bool routing = true;
    

        logger.stamp().set("Starting internal router...").info();

        while (routing)
        {
            ServerSocket slave_socket = self->server_socket->accept_connection();
            server_ip_port_t ipport;
            ipport.socket = slave_socket;
            ipport.flag = true;
            ipport.id = 0;
            self->others.push_back(ipport);
            pthread_create(&ipport.id, NULL, InternalRouter::handle_connection, (void *)&(self->others));

        }
    }


    return p_context;
}

void *InternalRouter::handle_connection(void *input)
{
    std::vector<server_ip_port_t> *others = (std::vector<server_ip_port_t> *)input;
    server_ip_port_t in = others->back();
    cli_logger logger = cli_logger(frontend.get_log_stream());

    while (in.flag)
    {
        packet p = in.socket.read_packet();

        //logger.stamp().set("new packet on router").info();
        std::string message = "";
        switch (p.type)
        {
        case packet_type::JOIN_REQ:
        {
            logger.stamp().set("Server join request").info();
            ServerSocket *ssock = (ServerSocket *)&(in.socket);
            sockaddr_in addr = ssock->cli_addr;
            in.server_ip = inet_ntoa(addr.sin_addr);
            in.server_port = atoi(p._payload);
            if(others->size() > 1)
            {
                for(int i = 0; i < others->size(); i+=2)
                {
                    const char *reply = std::string(others->at(i+1).server_ip + ":" + std::to_string(others->at(i+1).server_port)).c_str();
                    packet r = in.socket.build_packet(packet_type::JOIN_RESP, 0, 1, reply);
                    others->at(i).socket.write_packet(&r);
                }


                int new_backup_id = others->size() - 1;
                const char *reply = std::string(others->front().server_ip + ":" + std::to_string(others->front().server_port)).c_str();
                packet r = others->back().socket.build_packet(packet_type::JOIN_RESP, 0, 1, reply);
                others->back().socket.write_packet(&r);

                char * new_backup_id_str = std::to_string(new_backup_id).c_str();
                r = others->back().socket.build_packet(packet_type::JOIN_RESP, 0, 1, new_backup_id_str);
                others->back().socket.write_packet(&r);

            }
            break;
        }
        case packet_type::SERVER_KEEPALIVE:
        {
            logger.stamp().set("Keep alive received").info();
            packet r = in.socket.build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
            in.socket.write_packet(&r);
            break;
        }
        case packet_type::JOIN_RESP:
        {
            std::string ip_port = std::string(p._payload);
            in.server_ip = ip_port.substr(0, ip_port.rfind(":"));
            in.server_port = atoi(ip_port.substr(ip_port.rfind(":")).c_str());
        }
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
    fd_set input_set;
    struct timeval timeout;
    packet p = sock->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
    while (!(*(ts->timeout)))
    {
        try{
            FD_ZERO(&input_set);
            FD_SET(sock->sockfd, &input_set);
            timeout.tv_sec = 0;
            timeout.tv_usec = TIMEOUTMS;
            sock->write_packet(&p);
            int n = select(sock->sockfd+1, &input_set, NULL, NULL, &timeout);
            if(n > 0)
            {

                packet r = sock->read_packet();
                switch(r.type)
                {
                    case packet_type::SERVER_KEEPALIVE:
                    {
                        *start = time(NULL);
                        logger.set(std::string("keepalive ") + std::to_string(*(ts->timeout))).stamp().info();
                        break;
                    }
                    case packet_type::UPLOAD_REQ:
                    {
                        *start = time(NULL);
                        //upload with additional username
                        break;
                    }
                    case packet_type::DELETE_REQ:
                    {
                        *start = time(NULL);
                        //delete with additional username
                        break;
                    }
                    case packet_type::LOGIN_REQ:
                    {
                        *start = time(NULL);
                        //login with additional username
                        break;
                    }
                }
                usleep(100);
            }
            else
            {
                logger.set("timeout").stamp().error();
                return NULL;
            }
        }
        catch(SocketError err){
            usleep(100);
            continue;
        }
    }
    logger.set("timeout").stamp().error();
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
    while ((time(NULL) - *start)*1000 < TIMEOUTMS)
    {
        usleep(1000);
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

int InternalRouter::get_next_backup_id() {
    return this->next_backup_id++;
}