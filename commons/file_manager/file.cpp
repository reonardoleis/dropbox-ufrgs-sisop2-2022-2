#include "file.hpp"
#include <fstream>
#include "../ui/cli_types.hpp"

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
    if(!file_stream.good())
    {
        file_stream.close();
        return -1;
    }
    file_stream.close();
    this->file_size = file_size;
    return 0;
}

int File::write_file(std::string &path) {
    std::string write_path = path + "/" + this->filename;
    printf("csminho: %s\n", write_path.c_str());
    cli_logger logger = cli_logger(frontend.get_log_stream());
    logger.set("writing file to " + write_path).stamp().warning();
    std::fstream file_stream = std::fstream(write_path, std::ios::out | std::ios::binary);
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
    bzero(file.filename, 256);
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
    serialized_file_t serialized_file = this->serialize();
    char * data = new char[this->file_size + sizeof(int) + 256];
    bzero(data, this->file_size + sizeof(int) + 256);
    printf("filesize %d\n", serialized_file.file_size);
    memcpy(data, (char *) &serialized_file.file_size, sizeof(int));
    memcpy(data + sizeof(int), (char *) serialized_file.filename, 256);
    memcpy(data + sizeof(int) + 256, (char *) serialized_file.data, this->file_size);

    return data;
}

serialized_file_t File::from_data(const char * data) {
    
    serialized_file_t file;
   
    memcpy((char *) &file.file_size, data, sizeof(int)); // copy filesize

    file.data = new char[file.file_size];
    memcpy((char *) file.filename, data + sizeof(int), 256); // copy filename
    memcpy((char *) file.data, data + sizeof(int) + 256, file.file_size); // copy data
 

    return file;
}


int File::get_payload_size() {
    return this->file_size + sizeof(int) + 256;
}