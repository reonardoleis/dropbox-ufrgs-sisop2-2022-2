#pragma once

#include <string>
#include <ctime>
#include <iostream>
#include <fstream>

class cli_message
{
    protected:
        std::string datestamp;
        std::string message;
        bool stamped;
    public:
        void set(std::string message);
        void stamp();
        int log(std::ofstream &file);
        virtual void print(std::ostream &out);
};

class cli_info: public cli_message
{
    public:
        void print(std::ostream &out);
};

class cli_warning: public cli_message
{
    public:
        void print(std::ostream &out);
};

class cli_error: public cli_message
{
    public:
        void print(std::ostream &out);
};