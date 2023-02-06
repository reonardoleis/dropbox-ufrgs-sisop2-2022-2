#include "input_manager.hpp"

InputManager::InputManager(Socket *soc)
{
    client_soc = soc;
    waiting = false;
    done = false;
    sem_init(&await, 0, 0);
}

packet InputManager::await_response()
{
    this->waiting_lock.lock();
    this->waiting = true;
    this->waiting_lock.unlock();
    sem_wait(&await);
    this->waiting_lock.lock();
    this->waiting = false;
    packet p = this->client_soc->read_packet();
    this->waiting_lock.unlock();
    return p;

}
void InputManager::give_response()
{
    sem_post(&await);
}
bool InputManager::is_waiting()
{
    this->waiting_lock.lock();
    bool ret = waiting;
    this->waiting_lock.unlock();
    return ret;
}
bool InputManager::is_done()
{
    this->done_lock.lock();
    bool ret = done;
    this->done_lock.unlock();
    return ret;
}

bool InputManager::should_send()
{
    this->send_lock.lock();
    bool ret = send;
    this->send_lock.unlock();
    return ret;
}

packet InputManager::get_packet()
{
    this->send_lock.lock();
    send = false;
    packet ret = *(this->pac);
    this->send_lock.unlock();
    return ret;
}

void InputManager::set_packet(packet *p)
{
    this->send_lock.lock();
    send = true;
    this->pac = new packet(*p);
    this->send_lock.unlock();
}

void InputManager::run()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    std::string base_path = std::string(homedir) + "/.dropboxUFRGS";
    std::string sync_dir = "/sync_dir";
    FileManager file_manager;
    std::string command, arg;
    bool download=false, remote = false, deleting=false, running = true;

    //initial sync_dir
    if(file_manager.create_directory(sync_dir) < 0)
    {
        printf("Local: Failed to create local sync_dir\n");
    }

    packet p = client_soc->build_packet_sized(packet_type::SYNC_DIR_REQ, 0, 1, 1, "");
    this->set_packet(&p);
    packet response = this->await_response();
    printf("Received: %s\n", response._payload);


    while (running) {
        uint16_t _packet_type = -1;
        std::string path = "./";

        printf("Enter the command: ");
        command = "";
        arg = "";
        int size = 1;
        const char * buf = "";

        getline(std::cin, command);
        int command_delim = command.find(" ");
        arg = command.substr(command_delim+1, command.length() - command_delim);
        command = command.substr(0, command_delim);
        printf("command: %s | arg: %s\n", command.c_str(), arg.c_str());


        remote = true;

        if (command.compare("get_sync_dir") == 0) {
            _packet_type = packet_type::SYNC_DIR_REQ;
            if(file_manager.create_directory(sync_dir) < 0)
            {
                printf("Local: Failed to create local sync_dir\n");
            }
        }

        if (command.compare("stop") == 0) {
            _packet_type = packet_type::STOP_SERVER_REQ;
            running=false;
        }

        if (command.compare("exit") == 0) {
            _packet_type = packet_type::LOGOUT_REQ;
            running=false;
        }

        
        if (command.compare("upload") == 0) {
            _packet_type = packet_type::UPLOAD_REQ;
            int delim = arg.rfind("/");
            File to_upload = File(arg.substr(delim+1));
            std::string filepath = arg.substr(0, delim);
            to_upload.read_file(filepath);
            buf = to_upload.to_data();
            size = to_upload.get_payload_size();
        }

        if (command.compare("download") == 0) {
            _packet_type = packet_type::DOWNLOAD_REQ;
            buf = arg.c_str();
            size = arg.length();
            download = true;
        }

        if (command.compare("delete_safe") == 0) {
            deleting = true;
            _packet_type = packet_type::DELETE_REQ;
            buf = arg.c_str();
            size = arg.length();
        }

        if (command.compare("delete") == 0) {
            _packet_type = packet_type::DELETE_REQ;
            buf = arg.c_str();
            size = arg.length();
            std::string file = base_path + sync_dir + "/" + arg;
            if(remove(file.c_str()) < 0)
            {
                printf("Local: Failed to remove file");
            }
            else
            {
                printf("Local: Successfully removed file");
            }
        }

        if (command.compare("list_server") == 0) {
            _packet_type = packet_type::LIST_REQ;
            buf = "";
            size = 1;
        }

        if (command.compare("list_client") == 0) {
            remote = false;
            std::string out;
            std::string path = base_path + sync_dir;
            if(file_manager.list_directory(path, out) == -1)
            {
                printf("Failed to list directory\n");
            }
            else
            {
                printf("local: %s", out.c_str());
            }


        }

        if(remote)
        {
            packet p = client_soc->build_packet_sized(_packet_type, 0, 1, size, buf);
            this->set_packet(&p);
            packet response = this->await_response();
            if(deleting)
            {
                deleting = false;
                std::string file = base_path + sync_dir + "/" + arg;
                printf("Received: %s\n", response._payload);
                if(response.type == packet_type::DELETE_REFUSE_RESP)
                {
                printf("Local: Delete aborted to keep data consistency\nTo delete independently from server use \'delete <filename.ext>\'\n");
                }
                else if(remove(file.c_str()) < 0)
                {
                printf("Local: Failed to remove file\n");
                }
                else
                {
                printf("Local: Successfully removed file\n");
                }
            }
            if(download)
            {
                download = false;
                serialized_file_t sf = File::from_data(response._payload);
                File write_file(sf);
                printf("Received: %s\n", write_file.filename.c_str());
                std::string path = base_path + sync_dir;
                if(write_file.write_file(path) < 0)
                {
                printf("Local: Failed writing received file\n");
                }
            }
            else
            {
                printf("Received: %s\n", response._payload);
            }
        }
    }
    this->done_lock.lock();
    this->done = true;
    this->done_lock.unlock();
}

void * InputManager::thread_ready(void * manager)
{
    InputManager* sm = (InputManager *)manager;
    sm->run();
}