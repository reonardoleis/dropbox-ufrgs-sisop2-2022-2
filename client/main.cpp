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
  bool running = true, download=false, remote = false;
    
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


  while (running) {
    uint16_t package_type = 0;
    std::string path = "./";

    printf("Enter the command: ");
    command = "";
    arg = "";
    int size = 0;
    const char * buf = "";

    std::cin >> command;
    std::cin >> arg;

    remote = true;

    if (command.compare("get_sync_dir") == 0) {
      package_type = packet_type::SYNC_DIR_REQ;
      if(file_manager.create_directory(sync_dir) < 0)
      {
        printf("Failed to create local sync_dir\n");
      }
    }

    if (command.compare("stop") == 0) {
      package_type = packet_type::STOP_SERVER_REQ;
    }

    if (command.compare("exit") == 0) {
      package_type = packet_type::LOGOUT_REQ;
      running = false;
    }

    
    if (command.compare("upload") == 0) {
      package_type = packet_type::UPLOAD_REQ;
      int delim = arg.rfind("/");
      File to_upload = File(arg.substr(delim+1, arg.length()));
      std::string filepath = arg.substr(0, delim);
      to_upload.read_file(filepath);
      buf = to_upload.to_data();
      size = to_upload.get_payload_size();
    }

    if (command.compare("download") == 0) {
      package_type = packet_type::DOWNLOAD_REQ;
      buf = arg.c_str();
      size = strlen(buf);
      download = true;
    }

    if (command.compare("list_server") == 0) {
      package_type = packet_type::LIST_REQ;
      buf = "";
      size = 1;
    }

    if (command.compare("list_client") == 0) {
      remote = false;
      std::string out;
      std::string path = base_path + sync_dir;
      if(file_manager.list_directory(path, out) == -1)
      {
        printf("Failed to list directory\n");
      }
      else
      {
        printf("local: %s", out.c_str());
      }


    }

    if(remote)
    {
      packet p2 = client_soc.build_packet_sized(package_type, 0, 1, size, buf);
      client_soc.write_packet(&p2);
      packet response = client_soc.read_packet();
      printf("Received: %s\n", response._payload);
    }
    


    
    if (download)
    {
      download = false;
      serialized_file_t sf = File::from_data(response._payload);
      File write_file(sf);
      std::string path = base_path + sync_dir;
      if(write_file.write_file(path) < 0)
      {
        printf("Failed writing recieved file\n");
      }
    }
  }
  
  sleep(1);
  client_soc.close_connection();

  return 0;
}