#include "internal_router.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

InternalRouter::InternalRouter(ServerSocket *server_socket)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
    this->next_backup_id = 0;
}

InternalRouter::InternalRouter(ServerSocket *server_socket, ConnectionsManager *connections_manager)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
    this->connections_manager = connections_manager;
    this->next_backup_id = 0;
}

InternalRouter::InternalRouter(ServerSocket *server_socket, BackupClientSocket *client_socket)
{
    this->server_socket = server_socket;
    this->router = new Router(server_socket);
    this->client_socket = client_socket;
    this->next_backup_id = 0;
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
    while(!self->get_is_master()) {

        logger.stamp().set("starting connections manager because I'm not the master").info();
        pthread_create(&keep_alive_thread_id, NULL, InternalRouter::keepalive, input);
        *(ts.start) = time(NULL);
        //pthread_create(&timeout_thread_id, NULL, InternalRouter::timeout, (void *)&ts);
        //usleep(100);
        //pthread_join(timeout_thread_id, NULL);
        pthread_join(keep_alive_thread_id, NULL);
        logger.set("Starting voting round").stamp().warning();

        // Voting round to determine new master;
        if(self->get_adjacent().server_ip.empty())
        {
            self->set_is_master(true);
            return p_context;
        }
        bool voting = true;
        self->client_socket->close_connection();
        server_ip_port_t ipport = self->get_adjacent();
        self->client_socket->reset_connection(ipport.server_ip.c_str(), ipport.server_port + 50);
        std::string ip_port_str = self->get_ip() + std::string(":") + std::to_string(self->server_socket->port);
        packet my_vote = self->client_socket->build_packet(packet_type::VOTE, 0, self->get_id(), ip_port_str.c_str());
        ServerSocket *my_adjacentsocket = new ServerSocket(8080, 2);
        try{ 
            self->client_socket->connect_to_server();
            *my_adjacentsocket = self->server_socket->accept_connection();
        }
        catch(SocketError err)
        {
            *my_adjacentsocket = self->server_socket->accept_connection();
            self->client_socket->connect_to_server();
        }
        self->client_socket->write_packet(&my_vote);

        while (voting) {
            packet p = my_adjacentsocket->read_packet();
            if(p.type == packet_type::VOTE)
            {
                logger.stamp().set("Received vote").info();
                int id_int = p.total_size;
                if(p.seqn == 1)
                {
                    try{
                        self->client_socket->write_packet(&my_vote);
                    }
                    catch(SocketError err){}
                    voting = false;
                    std::string ip_port = std::string(p._payload);
                    std::string server_ip = ip_port.substr(0, ip_port.rfind(":"));
                    int server_port = atoi(ip_port.substr(ip_port.rfind(":")).c_str());
                    self->client_socket->reset_connection(server_ip.c_str(), server_port);
                    //self->client_socket->connect_to_server();
                }
                else if(p.seqn == 0)
                {
                    if (self->get_id() == id_int) {
                        logger.stamp().set("I'm the master >:)").info();
                        self->set_is_master(true);
                        p.seqn = 1;
                        self->client_socket->write_packet(&p);
                        self->client_socket->close_connection();
                        voting = false;
                    } else if (self->get_id() < id_int) {
                        logger.stamp().set("I want to be the master").info();
                        self->client_socket->write_packet(&my_vote);
                    } else {
                        logger.stamp().set("I don't want to be the master").info();
                        self->client_socket->write_packet(&p);
                    }
                }
            }
        }
    }
    if(self->get_is_master())
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
    server_ip_port_t &in = others->back();
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
            ServerSocket *ssock = (ServerSocket *)&(in.socket);
            sockaddr_in addr = ssock->cli_addr;
            in.server_ip = inet_ntoa(addr.sin_addr);
            in.server_port = atoi(p._payload);
            logger.stamp().set(std::string("Server join request from ") + std::string(others->back().server_ip + ":" + std::to_string(others->back().server_port))).info();
            if(others->size() > 1)
            {
                const char *reply1 = std::string(others->back().server_ip + ":" + std::to_string(others->back().server_port)).c_str();
                packet r1 = in.socket.build_packet(packet_type::JOIN_RESP, 0, 1, reply1);
                others->at(others->size()-2).socket.write_packet(&r1);
                logger.stamp().set(std::string("Joining ") + std::string(others->at(others->size()-2).server_ip + ":" + std::to_string(others->at(others->size()-2).server_port))).info();


                const char *reply2 = std::string(others->front().server_ip + ":" + std::to_string(others->front().server_port)).c_str();
                packet r2 = others->back().socket.build_packet(packet_type::JOIN_RESP, 0, 1, reply2);
                others->back().socket.write_packet(&r2);
                logger.stamp().set(std::string("Joining ") + std::string(others->back().server_ip + ":" + std::to_string(others->back().server_port))).info();


            }
            int new_backup_id = others->size() - 1;
            const char * new_backup_ip_str = in.server_ip.c_str();
            logger.stamp().set(std::string("Sending id ") + std::to_string(new_backup_id) + " with " + std::string(in.server_ip)).info();
            packet r3 = others->back().socket.build_packet(packet_type::YOUR_ID_RESP, 0, new_backup_id, new_backup_ip_str);
            others->back().socket.write_packet(&r3);
            break;
        }
        case packet_type::SERVER_KEEPALIVE:
        {
            //logger.stamp().set("Keep alive received").info();
            packet r = in.socket.build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
            in.socket.write_packet(&r);
            break;
        }
        }
    }

    return NULL;
}

void * InternalRouter::keepalive(void * input)
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    InternalRouter *self = (InternalRouter *)input;
    BackupClientSocket *sock  = self->client_socket;
    fd_set input_set;
    struct timeval timeout;
    packet p = sock->build_packet(packet_type::SERVER_KEEPALIVE, 0, 1, "");
    bool timeout_b = false;
    while (!timeout_b)
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
                        break;
                    }
                    case packet_type::UPLOAD_REQ:
                    {
                        //upload with additional username
                        break;
                    }
                    case packet_type::DELETE_REQ:
                    {
                        //delete with additional username
                        break;
                    }
                    case packet_type::LOGIN_REQ:
                    {
                        //login with additional username
                        break;
                    }
                    case packet_type::JOIN_RESP:
                    {
                        char * ip_port = r._payload;
                        std::string ip_port_str = std::string(ip_port);
                        logger.stamp().set(std::string("Received join response ") + ip_port_str).info();
                        std::string ip = ip_port_str.substr(0, ip_port_str.find(":"));
                        std::string port_str = ip_port_str.substr(ip_port_str.find(":") + 1, ip_port_str.length());
                        int port = std::stoi(port_str);
                        server_ip_port_t adjacent;
                        adjacent.server_ip = ip;
                        adjacent.server_port = port;
                        self->set_adjacent(adjacent);
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
            logger.set("Error comunicationg with master server").stamp().error();
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

void InternalRouter::set_id(int id)
{
    this->_id = id;
}

int InternalRouter::get_id()
{
    return this->_id;
}

void InternalRouter::set_adjacent(server_ip_port_t adjacent)
{
    this->adjacent = adjacent;
}

server_ip_port_t InternalRouter::get_adjacent()
{
    return this->adjacent;
}

void InternalRouter::set_ip(const char *ip)
{
    this->external_ip = ip;
}

std::string InternalRouter::get_ip()
{
    return this->external_ip;
}