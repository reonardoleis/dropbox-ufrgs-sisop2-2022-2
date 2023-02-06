#include "sync_manager.hpp"

SyncManager::SyncManager()
{
    this->has_event = false;
    this->should_stop = false;
    this->events_amount = 0;
    this ->fd = inotify_init();

}

void SyncManager::watch(std::string filepath)
{
    inotify_add_watch(this->fd, filepath.c_str(), IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE | IN_MODIFY | IN_CREATE);
}


bool SyncManager::has_events()
{
    this->event_lock.lock();
    bool ret = this->has_event;
    this->event_lock.unlock();
    return ret;
}

int* SyncManager::get_events()
{
    this->event_lock.lock();
    int *ret = new int[this->events_amount];
    for(int i = 0; i < this->events_amount; i++)
    {
        ret[i] = this->events[i];
    }
    return ret;
}

int SyncManager::get_amount()
{
    int ret = this->events_amount;
    this->events_amount = 0;
    this->event_lock.unlock();
    return ret;
}

void SyncManager::run()
{
    char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    while(!this->get_stop())
    {
        int len = 0;
        while((len <= 0) && (errno == EAGAIN) && !this->get_stop())
        {
            len = read(this->fd, buffer, sizeof(buffer));
            usleep(100);
        }
        if(len <= 0)
        {
            break;
        }
        int count = 0;
        for (char *ptr = buffer; ptr < buffer + len;ptr += sizeof(struct inotify_event) + event->len) 
        {
            int flags[5] = {};
            int local_count = 0;

            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_MOVED_FROM)
            {
                flags[local_count] = IN_MOVED_FROM;
                local_count += 1;
            }
            if (event->mask & IN_MOVED_TO)
            {
                flags[local_count] = IN_MOVED_TO;
                local_count += 1;
            }
            if (event->mask & IN_DELETE)
            {
                flags[local_count] = IN_DELETE;
                local_count += 1;
            }
            if (event->mask & IN_MODIFY)
            {
                flags[local_count] = IN_MODIFY;
                local_count += 1;
            }
            if (event->mask & IN_CREATE)
            {
                flags[local_count] = IN_CREATE;
                local_count += 1;
            }

            if(local_count > 0)
            {
                this->event_lock.lock();
                for(int i = 0; i < local_count; i++)
                {
                    this->events[this->events_amount] = flags[i];
                    this->events_amount += 1;
                }
                this->event_lock.unlock();
            }
        }   
    }
}

void SyncManager::stop()
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

void * SyncManager::thread_ready(void * manager)
{
    SyncManager* sm = (SyncManager *)manager;
    sm->run();
}