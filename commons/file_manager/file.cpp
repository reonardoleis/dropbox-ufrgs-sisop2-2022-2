#include "file.hpp"
#include <fstream>

File::File(const std::string& filename)
{
    this->filename = filename;
    this->data = NULL;
}

File::File(serialized_file_t file)
{
    this->deserialize(file);
}

int File::read_file() {
    std::fstream file_stream = std::fstream(this->filename, std::ios::in | std::ios::binary);
    if (!file_stream.is_open())
    {
        return -1;
    }

    file_stream.seekg(0, std::ios::end);
    int file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    this->data = new char[file_size];
    file_stream.read(this->data, file_size);
    file_stream.close();
    this->file_size = file_size;
    return 0;
}

int File::write_file(std::string &path) {
    std::string write_path = path + "/" + this->filename;
    std::fstream file_stream = std::fstream(write_path, std::ios::in | std::ios::binary);
    if (!file_stream.is_open())
    {
        return -1;
    }

    file_stream.write(this->data, this->file_size);
    file_stream.close();

    return 0;
}

serialized_file_t File::serialize() {
    serialized_file_t file;
    file.file_size = this->file_size;
    strcpy(file.filename, this->filename.c_str());
    file.data = new char[this->file_size];
    memcpy(file.data, this->data, this->file_size);
    return file;
}

void File::deserialize(serialized_file_t file) {
    this->file_size = file.file_size;
    this->filename = file.filename;
    this->data = new char[this->file_size];
    memcpy(this->data, file.data, this->file_size);
}

char * File::to_data() {
    char * data = new char[this->file_size + sizeof(serialized_file_t) - sizeof(char *)];
    serialized_file_t file = this->serialize();
    memcpy(data, &file, sizeof(serialized_file_t) - sizeof(char *));
    memcpy(data + sizeof(serialized_file_t) - sizeof(char *), file.data, file.file_size);
    return data;
}

serialized_file_t File::from_data(char * data) {
    serialized_file_t file;
    memcpy(&file, data, sizeof(serialized_file_t) - sizeof(char *));
    file.data = new char[file.file_size];
    memcpy(file.data, data + sizeof(serialized_file_t) - sizeof(char *), file.file_size);
    return file;
}

