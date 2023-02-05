#include "ui_template.hpp"

ui_template frontend;

ui_template::ui_template()
{
    this->refresh_size();
}

void ui_template::run_ui()
{
    pthread_mutex_lock(&(this->stop_lock));
    this->should_stop = false;
    pthread_mutex_unlock(&(this->stop_lock));
    this->clear();
    while(!should_stop || should_update)
    {
        pthread_mutex_lock(&(this->stop_lock));

        this->refresh_size();
        this->mov_cursor(0, 0);
        if(should_update)
        {
            pthread_mutex_lock(&(this->write_lock));
            this->should_update = false;

            this->frame_stream << this->log_stream.str() << this->console_stream.str();
            this->log_stream.clear();
            this->console_stream.clear();

            pthread_mutex_unlock(&(this->write_lock));
            std::cout << this->frame_stream.str();
        }
         
        pthread_mutex_unlock(&(this->stop_lock));
        usleep(100);
    }
}

void ui_template::stop_ui()
{
    pthread_mutex_lock(&(this->stop_lock));
    this->should_stop = true;
    pthread_mutex_unlock(&(this->stop_lock));
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

void ui_template::hold_write_lock()
{
    pthread_mutex_lock(&(this->write_lock));
}

void ui_template::release_write_lock()
{
    pthread_mutex_unlock(&(this->write_lock));
    this->should_update = true;
}

