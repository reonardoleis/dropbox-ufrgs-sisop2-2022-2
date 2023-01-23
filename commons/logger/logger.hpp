#pragma once
#include <cstddef>

namespace logger
{
    void log(const char *message, const char *file_and_method = NULL);
    void warn(const char *message, const char *file_and_method = NULL);
    void error(const char *message, const char *file_and_method = NULL);
}
