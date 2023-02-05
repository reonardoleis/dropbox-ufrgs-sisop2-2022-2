#pragma once
#include <string>
#include <string.h>

typedef struct serialized_file_t {
    int file_size;
    char filename[256];
    char * data;
} serialized_file_t;

class File {
    public:
        File(serialized_file_t file);
        File(const std::string& filename);
        char * data;
        std::string filename;
        int read_file();
        int write_file(std::string &path);
        int file_size;
        serialized_file_t serialize();
        void deserialize(serialized_file_t file); 
        char * to_data();
        static serialized_file_t from_data(const char * data);
        int get_payload_size();
};