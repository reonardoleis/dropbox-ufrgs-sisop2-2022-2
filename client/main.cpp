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
#include "./sync_manager/sync_manager.hpp"


void *packet_listener(void *arg)
{

  std::string base_path = "./";
  std::string sync_dir = "/sync_dir";
  FileManager file_manager;

  ClientSocket *client_soc = (ClientSocket*) arg;


  while (true)
  {

    packet p = client_soc->read_packet();

    printf("type: %d\nlength: %d\nPayload: %s\n", p.type, p.length, p._payload);

    switch (p.type)
    {
    case packet_type::DELETE_ACCEPT_RESP:
    {
      std::string file = base_path + sync_dir + "/" + p._payload;
      printf("Received: successfully deleted %s\n", p._payload);
      if (remove(file.c_str()) < 0)
      {
        printf("Local: Failed to remove file\n");
      }
      else
      {
        printf("Local: Successfully removed file\n");
      }
      
      break;
    }
    case packet_type::DOWNLOAD_ACCEPT_RESP:
    {
      printf("attempting to download payload %s\n", p._payload + sizeof(int));

      serialized_file_t sf = File::from_data(p._payload);
      File write_file(sf);
      printf("Received: %s\n", write_file.filename.c_str());
      std::string path = base_path + sync_dir;
      if (write_file.write_file(path) < 0)
      {
        printf("Local: Failed writing received file\n");
      }
      break;
    }
    case packet_type::DELETE_REFUSE_RESP:
    {
      printf("Received: failed to delete file\n");
      break;
    }
    case packet_type::DOWNLOAD_REFUSE_RESP:
    {
      printf("Received: %s\n", p._payload);
      break;
    }
    default:
    {
      printf("Received: %s\n", p._payload);
    }
    }
  }
}

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


  SyncManager sync_manager(&client_soc);
  pthread_t sync_thread_id = 0;
  pthread_create(&sync_thread_id, NULL, SyncManager::thread_ready, &sync_manager);
  
  InputManager input_manager(&client_soc);
  inputmanager_input_t inman_input = {&input_manager, &sync_manager};
  pthread_t input_thread_id = 0;
  pthread_create(&input_thread_id, NULL, InputManager::thread_ready, &inman_input);


  pthread_t packet_listener_thread_id = 0;
  pthread_create(&packet_listener_thread_id, NULL, packet_listener, &client_soc);

  while (!input_manager.is_done())
  {
    usleep(100);
    if (input_manager.should_send())
    {
      packet p1 = input_manager.get_packet();
      //printf("type: %d\nlength: %d\nPayload: %s", p.type, p.length, p._payload);
      client_soc.write_packet(&p1);
    }
    if (sync_manager.should_send())
    {
      packet p2 = sync_manager.get_packet();
      //printf("type: %d\nlength: %d\nPayload: %s", p.type, p.length, p._payload);
      client_soc.write_packet(&p2);
    }
  }

  client_soc.close_connection();

  pthread_join(input_thread_id, NULL);
  pthread_join(sync_thread_id, NULL);
  pthread_join(packet_listener_thread_id, NULL);


  return 0;
}
