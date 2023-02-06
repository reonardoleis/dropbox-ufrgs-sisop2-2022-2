#pragma once
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <mutex>

class SyncManager
{
    private:
        int fd;
        int events[256];
        bool has_event;
        bool should_stop;
        std::mutex stop_lock;
        std::mutex event_lock;
        int events_amount;
    public:
        SyncManager();
        void watch(std::string filepath);
        static void * thread_ready(void * manager);
        bool has_events();
        int* get_events(); // should lock editing events
        int get_amount(); // should unlock editing events
        void run();
        void stop();
        bool get_stop();
};