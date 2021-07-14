
#ifndef STI_ENGINE_TICKET_H
#define STI_ENGINE_TICKET_H

#include <mutex>
#include <condition_variable>


namespace STI
{
namespace Engine
{


class Ticket
{
public:
    
    enum class TicketStatus { Running, Complete, Cancelled, NotFound };

    Ticket(const TicketStatus& initalState);
    virtual ~Ticket() {}

    void wait();
    void setComplete();
    void cancel();

    TicketStatus getStatus();

private:

    virtual bool waitCheck();

    TicketStatus status;

    mutable std::mutex statusMutex;
    mutable std::condition_variable statusCondition;
};


} //Engine
} //STI

#endif

