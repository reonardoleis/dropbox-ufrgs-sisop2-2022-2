#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#include "../commons/ui/cli_types.hpp"

#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    int port = 8080;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }
    cli_info info; info.set(std::string("test info"));
    cli_error error; info.set(std::string("test error"));
    cli_warning warning; info.set(std::string("test warning"));

    info.print(std::cout);    
    warning.print(std::cout); 
    error.print(std::cout); 
    cout << "starting server on port " << port << "\n";

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);

    Router router = Router(&master_socket);

    router.start();
 

    master_socket.close_connection();
  
    return 0;
}