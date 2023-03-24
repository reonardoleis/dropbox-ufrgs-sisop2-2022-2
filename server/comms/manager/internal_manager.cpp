//TODO: Complete internal_manager (ONLY NEEDED FOR MASTER)
/*#include "internal_manager.hpp"

InternalManager::InternalManager()
{
    this->is_router_routing = new bool(true);
}

InternalManager::InternalManager(bool *is_router_routing)
{
    this->is_router_routing = is_router_routing;
}

int InternalManager::add_backup(backup_t backup)
{
    this->backups.push_back(backup);
    return 0;
}

int InternalManager::broadcast(packet *p)
{
    for (int i = 0; i < this->backups.size(); i++)
    {
        this->backups[i].connection->write_packet(p);
    }
    return 0;
}

void * InternalManager::manage(void *manager)
{
    InternalManager *self = (InternalManager *) manager;
    cli_logger logger = cli_logger(frontend.get_log_stream());
    logger.stamp().set("Starting internal manager...").info();
    while (*self->is_router_routing)
    {
        for (int i = 0; i < self->backups.size(); i++)
        {
            if (self->backups[i].connection->get_is_waiting())
            {
                packet *p = self->backups[i].connection->read_packet();
                if (p->type == packet_type::SERVER_KEEPALIVE)
                {
                    logger.stamp().set("Received keep alive from backup").info();
                }
                else
                {
                    logger.stamp().set("Received unknown packet from backup").info();
                }
            }
        }
        sleep(1);
    }
    logger.stamp().set("Exiting internal manager...").info();
    return NULL;
}

void InternalManager::set_is_router_routing(bool *is_router_routing)
{
    this->is_router_routing = is_router_routing;
}

bool *InternalManager::get_is_router_routing()
{
    return this->is_router_routing;
}

*/