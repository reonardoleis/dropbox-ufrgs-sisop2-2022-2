#include <iostream>
#include "./comms/socket/server_socket.hpp"
#include "./comms/router/router.hpp"
#include "../commons/ui/cli_types.hpp"
#include "../commons/ui/ui_template.hpp"

#define MASTER_SOCKET_QUEUE_SIZE 5

using namespace std;
int main(int argc, char *argv[])
{
    cli_logger logger = cli_logger(frontend.get_log_stream());
    int port = 8080;
    if (argc >= 2)
    {
        port = atoi(argv[1]);
    }

    
    pthread_t ui_thread_id = 0;
    pthread_create(&ui_thread_id, NULL, ui_template::thread_ready, &frontend);
    logger.set("Starting server on port: " + to_string(port)).stamp().info();
   

    ServerSocket master_socket = ServerSocket(port, MASTER_SOCKET_QUEUE_SIZE);

    Router router = Router(&master_socket);

    router.start();
 

    master_socket.close_connection();
    logger.set("Closing server...").stamp().info();
    frontend.stop_ui();
    pthread_join(ui_thread_id, NULL);
    return 0;
}