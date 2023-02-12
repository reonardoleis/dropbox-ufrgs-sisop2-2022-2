#include "sync_manager.hpp"
#include <errno.h>
SyncManager::SyncManager(Socket *client_sock)
{
    this->send = false;
    this->should_stop = false;
    this->events_amount = 0;
    this->fd = inotify_init1(IN_NONBLOCK);
    this->wd = 0;
    this->client_soc = client_soc;
    sem_init(&watching, 0, 0);
}

void SyncManager::watch(std::string filepath)
{
    this->wd = inotify_add_watch(this->fd, filepath.c_str(), IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE | IN_MODIFY | IN_CREATE);
    if ((errno != ENOTDIR))
    {
        sem_post(&(this->watching));
        printf("%s", (errno != EEXIST) ? "Local: Watcher created\n" : "Local: Using existing watcher\n");
    }
    else
    {
        printf("ERROR: Path is not a directory\n");
    }
}

void SyncManager::run()
{
    char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    char cCurrentPath[FILENAME_MAX];
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        this->stop_sync();
        printf("Failed to get running directory: ERRNO %d", errno);
    }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    bool rename = false;
    sem_wait(&(this->watching));
    while (!this->get_stop())
    {
        int size = 0;
        char *message = NULL;
        uint16_t p_type = -1;
        int len = 0;
        while ((len <= 0) && !this->get_stop())
        {
            len = read(this->fd, buffer, sizeof(buffer));
            if (len == -1 && errno != EAGAIN)
            {
                printf("AQUI ERRO\n");
                exit(EXIT_FAILURE);
            }
            usleep(100);
        }
        if (len <= 0)
        {
            break;
        }
        int count = 0;
        for (char *ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len)
        {
            size = 0;
            event = (const struct inotify_event *)ptr;
            printf("EVENT MASK: %x\n", event->mask);
            if (event->mask & IN_MOVED_TO)
            {
                printf("NOTIFY CREATE\n");
                p_type = packet_type::UPLOAD_REQ;
                File f = File(event->name);
                std::string base_path = std::string(cCurrentPath);
                std::string sync_dir = base_path + std::string("/sync_dir");
                f.read_file(sync_dir);
                size = f.get_payload_size();
                message = f.to_data();
            }
            else if (event->mask & IN_MOVED_FROM)
            {
                p_type = packet_type::DELETE_REQ;
                size = event->len;
                message = (char *)event->name;
            }
            else if (event->mask & IN_DELETE)
            {
                printf("NOTIFY DELETE\n");
                p_type = packet_type::DELETE_REQ;
                size = event->len;
                message = (char *)event->name;
            }
            else if (event->mask & IN_MODIFY)
            {
                printf("NOTIFY MODIFY\n");
                p_type = packet_type::UPLOAD_REQ;
                File f = File(event->name);
                std::string base_path = std::string(cCurrentPath);
                std::string sync_dir = base_path + std::string("/sync_dir");
                f.read_file(sync_dir);
                size = f.get_payload_size();
                message = f.to_data();
            }
            else if (event->mask & IN_CREATE)
            {
                printf("NOTIFY CREATE\n");
                p_type = packet_type::UPLOAD_REQ;
                File f = File(event->name);
                std::string base_path = std::string(cCurrentPath);
                std::string sync_dir = base_path + std::string("/sync_dir");
                f.read_file(sync_dir);
                size = f.get_payload_size();
                message = f.to_data();
            }
            else if (event->mask & IN_CLOSE_WRITE)
            {
                printf("NOTIFY MODIFY\n");
                p_type = packet_type::UPLOAD_REQ;
                File f = File(event->name);
                std::string base_path = std::string(cCurrentPath);
                std::string sync_dir = base_path + std::string("/sync_dir");
                f.read_file(sync_dir);
                size = f.get_payload_size();
                message = f.to_data();
            }

            if (!rename && size > 0)
            {
                if (!this->get_should_ignore())
                {
                    printf("NOTIFY PACKET: %d | %s\n", size, event->name);
                    packet p = this->client_soc->build_packet_sized(p_type, 0, 0, size, message);
                    this->set_packet(&p);
                }
            }
        }

        this->should_ignore_lock.lock();
        this->should_ignore = false;
        this->should_ignore_lock.unlock();
    }

    inotify_rm_watch(this->fd, this->wd);
    close(this->fd);
}

packet SyncManager::get_packet()
{
    this->send_lock.lock();
    send = false;
    packet ret = *(this->pac);
    this->send_lock.unlock();
    return ret;
}

void SyncManager::set_packet(packet *p)
{
    this->send_lock.lock();
    send = true;
    this->pac = new packet(*p);
    this->send_lock.unlock();
    usleep(100);
}

void SyncManager::stop_sync()
{
    this->stop_lock.lock();
    this->should_stop = true;
    this->stop_lock.unlock();
}

bool SyncManager::get_stop()
{
    this->stop_lock.lock();
    bool ret = this->should_stop;
    this->stop_lock.unlock();
    return ret;
}

bool SyncManager::should_send()
{
    this->send_lock.lock();
    bool ret = send;
    this->send_lock.unlock();
    return ret;
}

void *SyncManager::thread_ready(void *manager)
{
    SyncManager *sm = (SyncManager *)manager;
    sm->run();
    return NULL;
}

bool SyncManager::get_should_ignore()
{
    this->should_ignore_lock.lock();
    bool ret = this->should_ignore;
    this->should_ignore_lock.unlock();
    return ret;
}

void SyncManager::ignore()
{
    this->should_ignore_lock.lock();
    this->should_ignore = true;
    this->should_ignore_lock.unlock();
}