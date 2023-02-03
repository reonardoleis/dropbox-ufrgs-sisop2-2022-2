#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#include "../commons/ui/cli_types.hpp"
#include "../commons/ui/ui_template.hpp"

#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    ui_template _ui_template;
    cli_logger logger = cli_logger();
    int port = 8080;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }

    
   // pthread_t ui_thread_id = 0;
   // pthread_create(&ui_thread_id, NULL, _ui_template.thread_ready, &_ui_template);
    logger.set("Starting server on port: " + to_string(port)).stamp().info();
   

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);

    Router router = Router(&master_socket);

    router.start();
 

    master_socket.close_connection();

    //pthread_join(ui_thread_id, NULL);
    return 0;
}