#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#include "../commons/ui/cli_types.hpp"

#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    cli_info info;
    int port = 8080;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }

    info.stamp(); info.set(std::string("server starting on port: ") + std::to_string(port));
    info.print(std::cout);    
   

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);

    Router router = Router(&master_socket);

    router.start();
 

    master_socket.close_connection();
  
    return 0;
}