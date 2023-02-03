#include "cli_types.hpp"


cli_logger cli_logger::set(std::string message)
{
    this->message = message;
    return *this;
}

cli_logger cli_logger::stamp()
{
    time_t epoch = (time_t)time(NULL);
    struct tm *local = localtime(&epoch);
    tzset();
    char buffer[26];
    strftime(buffer, 28, "[%Y-%m-%d %H:%M:%S] ", local);
    this->datestamp = std::string(buffer);

    return *this;
}

int cli_logger::log(std::ofstream &file)
{
    if (!file.is_open())
        return -1;

    file << this->datestamp << this->message << std::endl;
    return 0;
}


cli_logger cli_logger::info()
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[34m%sINFO:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    frontend.hold_write_lock();
    (this->stream) << buffer << std::endl;
    frontend.release_write_lock();
    return *this;
}

cli_logger cli_logger::warning()
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[33m%sWARNING:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    frontend.hold_write_lock();
    (this->stream) << buffer << std::endl;
    frontend.release_write_lock();
    return *this;
}

cli_logger cli_logger::error()
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[31m%sERROR:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    frontend.hold_write_lock();
    (this->stream) << buffer << std::endl;
    frontend.release_write_lock();
    return *this;
}