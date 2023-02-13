#include "input_manager.hpp"
#include <unistd.h>

InputManager::InputManager(Socket *soc)
{
    client_soc = soc;
    waiting = false;
    done = false;
    send = false;
    sem_init(&list_sem, 0, 0);
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
    this->send = true;
    this->pac = new packet(*p);
    this->send_lock.unlock();
    usleep(100);
}

void InputManager::run()
{
    bool download=false, remote = false, deleting=false, running = true;
    char cCurrentPath[FILENAME_MAX];
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        running = false;
        printf("Failed to get running directory: ERRNO %d", errno);
    }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';

    std::string base_path = cCurrentPath;
    std::string sync_dir = "/sync_dir";
    FileManager file_manager;
    file_manager.set_base_path(base_path);
    std::string command, arg;

    //initial sync_dir
    
    if(file_manager.create_directory(sync_dir) < 0)
    {
        printf("Local: Failed to create local sync_dir\n");
    }
    else
    {
        printf("Local: sync_dir created or already existed\n");
        packet lr = client_soc->build_packet_sized(packet_type::SYNC_DIR_REQ, 0, 0, 1, "");
        this->set_packet(&lr);
    }


    while (running) {
        uint16_t _packet_type = -1;

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
            if(file_manager.create_directory(sync_dir) < 0)
            {
                printf("Local: Failed to create local sync_dir\n");
            }
            else
            {
                this->sync_manager->watch(watch_path);
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
            printf("PATH: %s\n", path.c_str());
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


/*void InputManager::set_slist(std::string s)
{
    this->list_lock.lock();
    this->server_list = s;
    this->list_lock.unlock();
}
std::string InputManager::get_slist()
{
    this->list_lock.lock();
    std::string ret = this->server_list;
    this->list_lock.unlock();
    return ret;
}
void InputManager::wait_slist()
{
    sem_wait(&list_sem);
}
void InputManager::post_slist()
{
    sem_post(&list_sem);
}


int verify_sync(std::string server_files, std::string client_files, std::string &out)
{
    std::stringstream sfs;  sfs << server_files;
    std::stringstream cfs;  cfs << client_files;
    std::string smeta;
    std::string cmeta;
    std::string download_files;
    std::string upload_files;
    while(sfs.rdbuf()->in_avail() > 0)
    {
        std::getline(sfs, server_files);
        std::getline(sfs, smeta);
        if(cfs.rdbuf()->in_avail() > 0)
        {
            std::getline(cfs, client_files);
            std::getline(cfs, cmeta);
            int cmp = server_files.compare(client_files);
            if(cmp == 0)
            {
                int smod_start = smeta.find("\t", smeta.find("\t")+1);
                int smod_end = smeta.find("\t", smod_start + 1);
                std::string smod = smeta.substr(smod_start, smod_end);

                int cmod_start = cmeta.find("\t", cmeta.find("\t")+1);
                int cmod_end = cmeta.find("\t", cmod_start + 1);
                std::string cmod = cmeta.substr(cmod_start, cmod_end);

                int mcmp = smod.compare(cmod);
                if(mcmp > 0)
                {
                    download_files += server_files + ", ";
                }
                else if (cmp < 0)
                {
                    upload_files += client_files + ", ";
                }
            }
            else if(cmp < 0)
            {
                download_files += server_files + ", ";
            }
            else
            {
                upload_files += client_files + ", ";
            }
        }
        else
        {
            download_files += server_files + ", ";
        }
    }
    while(cfs.rdbuf()->in_avail() > 0)
    {
        std::getline(cfs, client_files);
        std::getline(cfs, cmeta);
        upload_files += client_files + ", ";
    }
    if(download_files.length > 0)
    {
        if(upload_files.length() > 0)
        {
            out = download_files + "|" + upload_files;
            return sync_type::MIXED;
        }
        else
        {
            out = download_files;
            return sync_type::DOWNLOAD;
        }
    }
    else if(upload_files.length() > 0)
    {
        out = upload_files;
        return sync_type::UPLOAD;
    }
    else
    {
        out = "";
        return sync_type::SYNCED;
    }
}*/