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
#include <pwd.h>
#include "../commons/packet.hpp"
#include "./comms/socket/client_socket.hpp"
#include "./comms/input_manager/input_manager.hpp"
#include "../commons/file_manager/file_manager.hpp"
#include "../commons/file_manager/file.hpp"


int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf("Wrong usage: expected ./myClient <username> <server_name_or_ip> <port>");
    return -1;
  }

  int port;
  std::string username;
  std::string server_address_str;
  std::string command, arg;
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;
  std::string base_path = std::string(homedir) + "/.dropboxUFRGS";
  std::string sync_dir = "/sync_dir";
  FileManager file_manager;

    
  username = argv[1];
  server_address_str = argv[2];
  port = atoi(argv[3]);

  file_manager.set_base_path(base_path);

  ClientSocket client_soc(server_address_str.c_str(), port);
  client_soc.connect_to_server();

  packet p = client_soc.build_packet(packet_type::LOGIN_REQ, 0, 1, username.c_str());
  client_soc.write_packet(&p);
  
  packet response = client_soc.read_packet();
  printf("response: %s\n", response._payload);

  InputManager input_manager(&client_soc);
  pthread_t input_thread_id = 0;
  pthread_create(&input_thread_id, NULL, InputManager::thread_ready, &input_manager);

  while (!input_manager.is_done()) {
    usleep(100);
    if(input_manager.should_send())
    {
      packet p = input_manager.get_packet();
      printf("type: %d\nlength: %d\nPayload: %s", p.type, p.length, p._payload);
      client_soc.write_packet(&p);
      if(input_manager.is_waiting())
      {
        input_manager.give_response();
      }
    }

  }
  
  sleep(1);
  client_soc.close_connection();

  return 0;
}

