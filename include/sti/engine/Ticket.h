#ifndef STI_ENGINE_TICKET_H
#define STI_ENGINE_TICKET_H

#include <mutex>
#include <condition_variable>
#include <functional>


namespace STI
{
namespace Engine
{


class Ticket
{
public:
    
    enum class TicketStatus { Running, Complete, Canceled, NotFound };

    Ticket(const TicketStatus& initalState);
    virtual ~Ticket() {}

    void wait();
    void wait(const std::function<bool()>& waitChecker);

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

