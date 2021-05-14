#ifndef STI_PYTHON_TICKETMANAGER_H
#define STI_PYTHON_TICKETMANAGER_H


#include "Device.h"
#include "SynchronizedMap.h"

#include <memory>
// #include <mutex>
#include <set>


namespace STI
{
namespace Python
{


template<typename ID, typename T>
class TicketManager 
{
public:

    TicketManager();
    ~TicketManager();

    void add(const ID& id, const std::shared_ptr<T>& ticket);
    void remove(const ID& id);
    bool get(const ID& id, std::shared_ptr<T>& ticket);

    void cancel(const ID& id);
    void cancelAll();

    std::shared_ptr<T> makeTicket(const ID& id, const std::shared_ptr<STI::Device::Device>& server);

private:

    // void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    STI::Utils::SynchronizedMap<ID, std::shared_ptr<T>> tickets;

};


} //Python
} //STI


template<typename ID, typename T>
STI::Python::TicketManager<ID, T>::TicketManager()
{
}

template<typename ID, typename T>
STI::Python::TicketManager<ID, T>::~TicketManager()
{
    cancelAll();
}

template<typename ID, typename T>
void STI::Python::TicketManager<ID, T>::add(const ID& id, const std::shared_ptr<T>& ticket)
{
    tickets.add(id, ticket);
}

template<typename ID, typename T>
void STI::Python::TicketManager<ID, T>::remove(const ID& id)
{
    tickets.remove(id);
}

template<typename ID, typename T>
bool STI::Python::TicketManager<ID, T>::get(const ID& id, std::shared_ptr<T>& ticket)
{
    return tickets.get(id, ticket);
}

template<typename ID, typename T>
void STI::Python::TicketManager<ID, T>::cancel(const ID& id)
{
    std::shared_ptr<T> ticket;

    if (tickets.get(id, ticket)) {
        ticket->cancel();
    }
}

template<typename ID, typename T>
void STI::Python::TicketManager<ID, T>::cancelAll()
{
    std::set<ID> ids;
    tickets.getKeys(ids);

    for(auto& id : ids) {
        cancel(id);
    }
}

template<typename ID, typename T>
std::shared_ptr<T> STI::Python::TicketManager<ID, T>::makeTicket(const ID& id, const std::shared_ptr<STI::Device::Device>& server)
{
    auto ticket = std::make_shared<T>(id, server);

    add(id, ticket);

    return ticket;
}


#endif

