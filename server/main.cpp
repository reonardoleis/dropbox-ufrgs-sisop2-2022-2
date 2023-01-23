#include <iostream>
#include "./comms/socket/server_socket.hpp"
#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    int port = 8080;
    if (argc >= 2) {
        port = atoi(argv[1]);
    }
    
    cout<<"starting server on port "<<port<<"\n";

    Socket master_socket = Socket(port, MASTER_SOCKET_QUEUE_SIZE);

    if (int err = master_socket.bind_and_listen() < 0) {
        return err;
    }


    Socket slave_socket = master_socket.accept_connection();
    packet p = slave_socket.read_packet();
    

    cout<<"message received:"<<p._payload<<"\nof type:"<<p.type<<"\n";

    packet p_response;


    char* message = "received";
    p_response._payload = (char*) malloc(strlen(message));
    memcpy(p_response._payload, message, strlen(message));

    int n = slave_socket.write_packet(&p_response);
    if (n < 0) {
        printf("error while writing packets");
        return n;
    }

	master_socket.close_connection();
	slave_socket.close_connection();
    return 0;
}