#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    int port = 8080;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }

    cout << "starting server on port " << port << "\n";

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);

    Router router = Router(&master_socket);

    router.start();
 

    master_socket.close_connection();
  
    return 0;
}