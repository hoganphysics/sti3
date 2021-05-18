
#ifndef STI_PYTHON_TICKET_H
#define STI_PYTHON_TICKET_H

#include <mutex>
#include <condition_variable>


namespace STI
{
namespace Python
{


class Ticket
{
public:
    
    enum class TicketStatus { Running, Complete, Cancelled };

    Ticket(const TicketStatus& initalState);
    virtual ~Ticket() {}

    void wait();
    void setComplete();
    void cancel();

    TicketStatus getStatus();

private:

    TicketStatus status;

    mutable std::mutex statusMutex;
    mutable std::condition_variable statusCondition;
};


} //Python
} //STI

#endif

