#include "logger.hpp"
#include <cstdio>
#include <ctime>
#include <chrono>
#include <cstring>

namespace logger
{
    void log(const char *message, const char *file_and_method)
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        char *date = ctime(&in_time_t);
        date[strlen(date) - 1] = '\0';

        if (file_and_method != NULL) {
            printf("[%s on %s]: %s\n", date, file_and_method, message);
            return;
        }
        printf("[%s]: %s\n", date, message);
    }

    void warn(const char *message, const char *file_and_method)
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        char *date = ctime(&in_time_t);
        date[strlen(date) - 1] = '\0';

        if (file_and_method != NULL) {
            printf("[%s on %s]: %s\n", date, file_and_method, message);
            return;
        }
        printf("\033[0;33m[%s]: %s\033[0m\n", date, message);
    }

    void error(const char *message, const char *file_and_method)
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        char *date = ctime(&in_time_t);
        date[strlen(date) - 1] = '\0';

        if (file_and_method != NULL) {
            printf("\033[1;31m[%s on %s]: %s\033[0m\n", date, file_and_method, message);
            return;
        }
        printf("\033[1;31m[%s]: %s\033[0m\n", date, message);
    }
}
