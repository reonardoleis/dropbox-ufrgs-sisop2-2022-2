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

  printf("Enter the message: ");
  std::string buffer;
  getline(std::cin, buffer);

  int package_type = 0;
  if (buffer.compare("stop") == 0) {
    package_type = packet_type::STOP_SERVER_REQ;
  }
  

  packet p = client_soc.build_packet(package_type, 0, 1, buffer.c_str());
  client_soc.write_packet(&p);
  
  packet response = client_soc.read_packet();
  printf("response: %s\n", response._payload);

  printf("Enter the message2: ");
  buffer = "";
  getline(std::cin, buffer);

  

  packet p2 = client_soc.build_packet(0, 0, 1, buffer.c_str());
  client_soc.write_packet(&p2);
  
  packet response2 = client_soc.read_packet();
  printf("response: %s\n", response2._payload);

  while (true) {
    packet response3 = client_soc.read_packet();
    printf("response3: %s\n", response3._payload);
  }
 

  client_soc.close_connection();

  return 0;
}