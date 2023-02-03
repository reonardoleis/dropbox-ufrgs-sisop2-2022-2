#include "cli_types.hpp"

void cli_message::set(std::string message)
{
    this->message = message;
}

void cli_message::stamp()
{
    time_t epoch = (time_t)time(NULL);
    struct tm *local = localtime(&epoch);
    tzset();
    char buffer[26];
    strftime(buffer, 28, "[%Y-%m-%d %H:%M:%S] ", local);
    this->datestamp = std::string(buffer);
    this->stamped = true;
}

int cli_message::log(std::ofstream &file)
{
    if (!file.is_open())
        return -1;

    file << this->datestamp << this->message << std::endl;
}

void cli_message::print(std::ostream &out)
{
    out << "not implemented" << std::endl;
}

void cli_info::print(std::ostream &out)
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[34m%sINFO:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    out << buffer << std::endl;
}

void cli_warning::print(std::ostream &out)
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[33m%sWARNING:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    out << buffer << std::endl;
}

void cli_error::print(std::ostream &out)
{
    char buffer[256];
    std::snprintf(buffer, 256, "\x1B[31m%sERROR:\033[0m %s", this->datestamp.c_str(), this->message.c_str());
    out << buffer << std::endl;
}