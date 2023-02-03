#pragma once
#include <iostream>
#include <sstream>

#include <sys/ioctl.h>
#include <unistd.h>

#define END_OF_TERM "\033[9999;1f"
#define START_OF_TERM "\033[1;1f"
#define CLEAR_TERM  "\033[2J"

class ui_template
{
    private:
        std::ostream log_stream;
        std::ostream console_stream;
        std::iostream frame_stream;
        struct winsize size;
    public:
        int get_term_columns();
        int get_term_lines();
        void refresh_size();
        void push_cursor_pos();
        void pop_cursor_pos();
        void clear();
        std::ostream& get_log_stream();
        std::ostream& get_console_stream();
        int mov_cursor(unsigned int lin, unsigned int col);
        virtual int run_ui();

        static void* thread_ready(void * ui);
};