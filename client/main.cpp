#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include "../commons/packet.hpp"
#include "./comms/socket/client_socket.hpp"
#include "../commons/file_manager/file_manager.hpp"
#include "../commons/file_manager/file.hpp"
int main(int argc, char *argv[])
{
  int port = 8080;
  if (argc >= 3)
  {
    port = atoi(argv[2]);
  }

  std::string server_address_str = "localhost";
  if (argc >= 2)
  {
    server_address_str = argv[1];
  }

  ClientSocket client_soc = ClientSocket(server_address_str.c_str(), port);
  client_soc.connect_to_server();

  printf("Enter your username: ");
  std::string buffer;
  getline(std::cin, buffer);
  packet p = client_soc.build_packet(packet_type::LOGIN_REQ, 0, 1, buffer.c_str());
  client_soc.write_packet(&p);
  
  packet response = client_soc.read_packet();
  printf("response: %s\n", response._payload);


  while (true) {
    uint16_t package_type = 0;
    File to_upload = File("test.txt");
    to_upload.read_file();

    printf("Enter the command: ");
    buffer = "";

    getline(std::cin, buffer);
    const char * buf = buffer.c_str();
    

    if (buffer.compare("sync_dir") == 0) {
      package_type = packet_type::SYNC_DIR_REQ;
    }

    if (buffer.compare("stop") == 0) {
      package_type = packet_type::STOP_SERVER_REQ;
    }

    if (buffer.compare("logout") == 0) {
      package_type = packet_type::LOGOUT_REQ;
    }

    if (buffer.compare("upload") == 0) {
      package_type = packet_type::UPLOAD_REQ;
      buf = to_upload.to_data();
      printf("buf: %s\n", buf + sizeof(serialized_file_t) - sizeof(char*)); 
    }

    

    packet p2 = client_soc.build_packet(package_type, 0, 1, buf);
    client_soc.write_packet(&p2);
     if (buffer.compare("stop") == 0) {
      //client_soc.close_connection();
      break;
    }

    
    packet response = client_soc.read_packet();
    printf("Received: %s\n", response._payload);
  }
  
  sleep(10);
  client_soc.close_connection();

  return 0;
}