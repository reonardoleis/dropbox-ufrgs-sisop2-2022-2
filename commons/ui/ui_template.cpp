#include "ui_template.hpp"

ui_template::ui_template()
{
    this->refresh_size();
}

int ui_template::run_ui()
{
    while(true)
    {
        this->refresh_size();
        this->clear();
        this->mov_cursor(0, 0);
        this->frame_stream << this->log_stream.rdbuf() << this->console_stream.rdbuf();
        std::cout << this->frame_stream.rdbuf();
        this->frame_stream.clear();
        sleep(200);
    }
}

int ui_template::mov_cursor(unsigned int lin, unsigned int col)
{
    char buffer[20] = {};
    if((lin <= this->get_term_lines()) && (col <= this->get_term_columns()))
    {
        snprintf(buffer, 20, "\033[%d;%df", lin, col);
        this->frame_stream << buffer;
        return 0;
    }
    return -1;
}

void ui_template::refresh_size()
{
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &(this->size));
}

void ui_template::push_cursor_pos()
{
    this->frame_stream << "\x1B[s";
}
void ui_template::pop_cursor_pos()
{
    this->frame_stream << "\x1B[u";
}

int ui_template::get_term_lines()
{
    return this->size.ws_row;
}

int ui_template::get_term_columns()
{
    return this->size.ws_col;
}

void * ui_template::thread_ready(void* ui)
{
    ((ui_template *)ui)->run_ui();
    return NULL;
}

void ui_template::clear()
{
    this->frame_stream << "\033[2J";
}

std::stringstream& ui_template::get_log_stream()
{
    return this->log_stream;
}

std::stringstream& ui_template::get_console_stream()
{
    return this->console_stream;
}

