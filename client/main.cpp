#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../types/packet.hpp"
#include "./comms/socket/client_socket.hpp"

int main(int argc, char *argv[])
{
  int port = 8080;
  if (argc >= 3)
  {
    port = atoi(argv[2]);
  }

  char *server_address = "localhost";
  if (argc >= 2)
  {
    server_address = argv[1];
  }

  ClientSocket client_soc = ClientSocket(server_address, port);
  client_soc.connect_to_server();

  char buffer[256];
  printf("Enter the message: ");
  bzero(buffer, 256);
  fgets(buffer, 256, stdin);
  
  packet p;
  p.type = 1;
  p.seqn = 1;
  p.total_size = 1;
  p.length = strlen(buffer);
  printf("p.length = %d", p.length);
  p._payload = (char *)malloc(p.length);

  int header_size = sizeof(packet) - sizeof(char *);
  memcpy(p._payload, buffer, strlen(buffer) * sizeof(char));

  char *final_buffer = (char *)malloc(256 + header_size);
  memcpy(final_buffer, (char *)&p, header_size);
  memcpy(final_buffer + header_size, p._payload, 256);

  client_soc.write_packet(&p);
  
  packet response = client_soc.read_packet();
  printf("response: %s", response._payload);

  client_soc.close_connection();

  return 0;
}