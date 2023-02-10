#include "sync_manager.hpp"
#include <errno.h>
SyncManager::SyncManager(Socket *client_sock)
{
    this->send = false;
    this->should_stop = false;
    this->events_amount = 0;
    this->fd = inotify_init();
    this->client_soc = client_soc;
    sem_init(&watching, 0, 0);

}

void SyncManager::watch(std::string filepath)
{
    inotify_add_watch(this->fd, filepath.c_str(), IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE | IN_MODIFY | IN_CREATE | IN_MASK_CREATE | IN_ONLYDIR);
    if((errno != ENOTDIR) && (errno != EEXIST))
    {
        sem_post(&(this->watching));
    }
    else
    {
        printf("ERROR: %d\n", errno == EEXIST);
    }
}


void SyncManager::run()
{
    char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    bool rename = false;
    sem_wait(&(this->watching));
    while(!this->get_stop())
    {
        printf("AQUI1\n");
        int size = 0;
        std::string message = "";
        uint16_t p_type = -1;
        int len = 0;
        while((len <= 0) && !this->get_stop())
        {
            printf("AQUI2\n");
            len = read(this->fd, buffer, sizeof(buffer));
            printf("LEN: %d\n", len);
            usleep(100);
            if (len == -1 && errno != EAGAIN) {
                    printf("AQUI ERRO\n");
                    exit(EXIT_FAILURE);
               }
        }
        printf("AQUI3\n");
        if(len <= 0)
        {
            break;
        }
        int count = 0;
        for (char *ptr = buffer; ptr < buffer + len;ptr += sizeof(struct inotify_event) + event->len) 
        {
            event = (const struct inotify_event *) ptr;
            printf("%d\n", event->mask);
            if ((event->mask & IN_MOVED_TO) && rename)
            {
                rename = false;
            }
            else if (event->mask & IN_MOVED_FROM)
            {
                if(event->cookie > 0)
                {
                    rename = true;
                }
                else
                {
                    p_type = packet_type::DELETE_REQ;
                    size = event->len;
                    message = event->name;
                }
            }
            else if (event->mask & IN_DELETE)
            {
                p_type = packet_type::DELETE_REQ;
                size = event->len;
                message = event->name;
                
            }
            else if (event->mask & IN_MODIFY)
            {
                p_type = packet_type::UPLOAD_REQ;
                size = event->len;
                message = event->name;
            }
            else if (event->mask & IN_CREATE)
            {
                p_type = packet_type::UPLOAD_REQ;
                size = event->len;
                message = event->name;
            }

            if(!rename)
            {
                packet p = this->client_soc->build_packet_sized(packet_type::DELETE_REQ, 0, 0, event->len, event->name);
                this->set_packet(&p);
            }
        }
    }
    printf("\n\nAQUI\n\n");
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

void * SyncManager::thread_ready(void * manager)
{
    SyncManager* sm = (SyncManager *)manager;
    sm->run();
    return NULL;
}