#pragma once

#include <string>
#include <ctime>
#include <iostream>
#include <fstream>

class cli_logger
{
    private:
        std::string datestamp;
        std::string message;
        std::ostream *stream;
        bool stamped;
    public:
        cli_logger set(std::string message);
        cli_logger stamp();
        int log(std::ofstream &file);
        cli_logger info();
        cli_logger warning();
        cli_logger error();
        cli_logger(std::ostream *stream);
        cli_logger();
};

