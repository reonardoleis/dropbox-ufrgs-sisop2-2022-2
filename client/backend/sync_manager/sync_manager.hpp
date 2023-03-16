#pragma once
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <mutex>
#include <semaphore.h>
#include "../../frontend/socket/client_socket.hpp"
#include "../../../commons/file_manager/file.hpp"

class SyncManager
{
    private:
        int fd;
        int wd;
        int events[256];
        bool send;
        bool should_stop;
        std::mutex stop_lock;
        std::mutex send_lock;
        sem_t watching;
        packet *pac;
        int events_amount;
        Socket *client_soc;
        std::mutex should_ignore_lock;
        bool should_ignore;

    public:
        SyncManager(Socket *client_socket);
        void watch(std::string filepath);
        static void * thread_ready(void * manager);
        bool should_send();
        packet get_packet();
        void set_packet(packet *p); 
        void run();
        void stop_sync();
        bool get_stop();
        bool get_should_ignore();
        void ignore();
};