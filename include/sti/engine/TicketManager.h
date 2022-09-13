#ifndef STI_ENGINE_TICKETMANAGER_H
#define STI_ENGINE_TICKETMANAGER_H


#include <sti/device/Device.h>
#include <sti/utils/SynchronizedMap.h>

#include <memory>
// #include <mutex>
#include <set>


namespace STI
{
namespace Engine
{


template<typename ID, typename T>
class TicketManager 
{
public:

    TicketManager();
    virtual ~TicketManager();

    void add(const ID& id, const std::shared_ptr<T>& ticket);
    void remove(const ID& id);
    bool get(const ID& id, std::shared_ptr<T>& ticket);

    void cancel(const ID& id);
    void cancelAll();

    std::set<ID> getIDs();

//    std::shared_ptr<T> makeTicket(const ID& id, const std::shared_ptr<STI::Device::Device>& server);

private:

    STI::Utils::SynchronizedMap<ID, std::shared_ptr<T>> tickets;

};


} //Engine
} //STI


template<typename ID, typename T>
STI::Engine::TicketManager<ID, T>::TicketManager()
{
}

template<typename ID, typename T>
STI::Engine::TicketManager<ID, T>::~TicketManager()
{
    cancelAll();
}

template<typename ID, typename T>
void STI::Engine::TicketManager<ID, T>::add(const ID& id, const std::shared_ptr<T>& ticket)
{
    tickets.add(id, ticket);
}

template<typename ID, typename T>
void STI::Engine::TicketManager<ID, T>::remove(const ID& id)
{
    tickets.remove(id);
}

template<typename ID, typename T>
bool STI::Engine::TicketManager<ID, T>::get(const ID& id, std::shared_ptr<T>& ticket)
{
    return tickets.get(id, ticket);
}

template<typename ID, typename T>
void STI::Engine::TicketManager<ID, T>::cancel(const ID& id)
{
    std::shared_ptr<T> ticket;

    if (tickets.get(id, ticket)) {
        ticket->cancel();
    }
    remove(id);
}

template<typename ID, typename T>
void STI::Engine::TicketManager<ID, T>::cancelAll()
{
    std::set<ID> ids;
    tickets.getKeys(ids);

    for(auto& id : ids) {
        cancel(id);
    }
}

template<typename ID, typename T>
std::set<ID> STI::Engine::TicketManager<ID, T>::getIDs()
{
    std::set<ID> ids;
    tickets.getKeys(ids);
    return ids;
}

// template<typename ID, typename T>
// std::shared_ptr<T> STI::Engine::TicketManager<ID, T>::makeTicket(const ID& id, const std::shared_ptr<STI::Device::Device>& server)
// {
//     auto ticket = std::make_shared<T>(id, server);

//     add(id, ticket);

//     return ticket;
// }


#endif

