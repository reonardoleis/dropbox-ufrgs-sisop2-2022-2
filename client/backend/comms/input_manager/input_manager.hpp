#pragma once
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
#include <semaphore.h>
#include "../../../../commons/packet.hpp"
#include "../../../frontend/socket/client_socket.hpp"
#include "../../../../commons/file_manager/file_manager.hpp"
#include "../../../../commons/file_manager/file.hpp"
#include "../../sync_manager/sync_manager.hpp"

enum sync_type:short {
    DOWNLOAD,
    UPLOAD,
    MIXED,
    SYNCED
};

class InputManager
{
    private:
    std::string server_list;
    packet *pac;
    std::mutex waiting_lock, done_lock, send_lock, list_lock;
    sem_t list_sem;
    bool waiting, done, send;
    Socket *client_soc;
    void set_packet(packet *p);
    
    public:
    SyncManager *sync_manager; 
    InputManager(Socket *soc);
    void run();
    static void* thread_ready(void* manager);
    bool is_waiting();
    bool is_done();
    bool should_send();
    packet get_packet();
    void set_slist(std::string s);
    std::string get_slist();
    void wait_slist();
    void post_slist();
};


typedef struct _inputmanager_input
{
    InputManager *inp_man;
    SyncManager *sync_manager;
} inputmanager_input_t;
