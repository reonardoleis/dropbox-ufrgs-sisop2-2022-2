#pragma once
#include ".././socket/backup_client_socket.hpp"
#include "../socket/server_socket.hpp"
#include "../../../commons/packet.hpp"
#include "../../../commons/ui/cli_types.hpp"
#include "../../../commons/ui/ui_template.hpp"

class ConnectionsManager {
    private:
        BackupClientSocket *backup_client_socket;
    public:
        ConnectionsManager(BackupClientSocket *backup_client_socket);
        static void * keep_alive(void * input);
        void set_backup_client_socket(BackupClientSocket *backup_client_socket);
        BackupClientSocket *get_backup_client_socket();
};