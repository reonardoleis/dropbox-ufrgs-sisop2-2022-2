#include "input_manager.hpp"

InputManager::InputManager(Socket *soc)
{
    client_soc = soc;
    waiting = false;
    done = false;
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
    std::string base_path = std::string(homedir);
    std::string sync_dir = "/sync_dir";
    FileManager file_manager;
    printf("CU1\n");
    file_manager.set_base_path(base_path);
    std::string command, arg;
    bool download=false, remote = false, deleting=false, running = true;

    printf("CU2\n");
    //initial sync_dir
    if(file_manager.create_directory(sync_dir) < 0)
    {
        printf("Local: Failed to create local sync_dir\n");
    }
    packet p = client_soc->build_packet_sized(packet_type::SYNC_DIR_REQ, 0, 1, 1, "");
    printf("CU3\n");
    this->set_packet(&p);

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
            std::string watch_path = base_path + sync_dir;
            if(file_manager.create_directory(watch_path) < 0)
            {
                printf("Local: Failed to create local sync_dir\n");
            }
            else
            {
                this->sync_manager->watch(sync_dir);
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
            size = arg.length() + 1;
            download = true;
        }

        if (command.compare("delete_safe") == 0) {
            deleting = true;
            _packet_type = packet_type::DELETE_REQ;
            buf = arg.c_str();
            size = arg.length() + 1;
        }

        if (command.compare("delete") == 0) {
            _packet_type = packet_type::DELETE_REQ;
            buf = arg.c_str();
            size = arg.length() + 1;
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
        }
    }
    this->sync_manager->stop_sync();
    this->done_lock.lock();
    this->done = true;
    this->done_lock.unlock();
}

void * InputManager::thread_ready(void * manager)
{
    inputmanager_input_t* in = (inputmanager_input_t *)manager;
    in->inp_man->sync_manager = in->sync_manager;
    in->inp_man->run();
    return NULL;
}
