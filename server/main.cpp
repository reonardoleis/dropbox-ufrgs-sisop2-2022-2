#include <iostream>
#include "./comms/socket/server_socket.hpp"
#define MASTER_SOCKET_PORT 8080
#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main()
{
    cout<<"starting server...\n";

    Socket master_socket = Socket(MASTER_SOCKET_PORT, MASTER_SOCKET_QUEUE_SIZE);

    if (int err = master_socket.bind_and_listen() < 0) {
        cout<<"ERROR on binding\n";
        return err;
    }


    Socket slave_socket = master_socket.accept_connection();
    slave_socket.read_packet();
    packet p = slave_socket.get_buffer();
    cout<<"message received:"<<p._payload<<"\nof type:"<<p.type<<"\n";

	master_socket.close_connection();
	slave_socket.close_connection();
    return 0;
}