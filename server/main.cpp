#include <iostream>
#include "./comms/socket/server_socket.hpp"
#define MASTER_SOCKET_PORT 8081
#define MASTER_SOCKET_QUEUE_SIZE 5

#include "../commons/logger/logger.hpp"

using namespace std;
int main()
{
    logger::log("starting server...");

    Socket master_socket = Socket(MASTER_SOCKET_PORT, MASTER_SOCKET_QUEUE_SIZE);

    if (int err = master_socket.bind_and_listen() < 0) {
        logger::error("error on binding");
        return err;
    }

    Socket slave_socket = master_socket.accept_connection();
    int n = slave_socket.read_packet();
    packet p = slave_socket.get_buffer();
    cout<<"------\n";
    cout<<"message received: "<<p._payload<<"\n";
    cout<<"of type: "<<p.type<<"\n";
    cout<<"number of bytes: "<<n<<"\n";

	master_socket.close_connection();
	slave_socket.close_connection();
    return 0;
}