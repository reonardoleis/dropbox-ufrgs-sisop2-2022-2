#pragma once
#include <iostream>
#include <sstream>

#include <sys/ioctl.h>
#include <unistd.h>



class ui_template
{
    protected:
        std::stringstream log_stream;
        std::stringstream console_stream;
        std::stringstream frame_stream;
        pthread_mutex_t write_lock;
        pthread_mutex_t stop_lock;
        bool should_stop;
        bool should_update;
        struct winsize size;
    public:
        ui_template();
        int get_term_columns();
        int get_term_lines();
        void refresh_size();
        void push_cursor_pos();
        void pop_cursor_pos();
        void clear();
        void hold_write_lock();
        void release_write_lock();
        std::stringstream& get_log_stream();
        std::stringstream& get_console_stream();
        int mov_cursor(unsigned int lin, unsigned int col);
        void stop_ui();
        virtual void run_ui();

        static void* thread_ready(void * ui);
};

extern ui_template frontend;

