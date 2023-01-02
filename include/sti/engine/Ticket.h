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
    
    enum class TicketStatus { Running, Complete, Canceled, NotFound, Deferred };

    Ticket(const TicketStatus& initalState);
    virtual ~Ticket() {}

    void wait() const;
    void wait(const std::function<bool()>& waitChecker) const;

    void setComplete();
    void cancel();
    void defer();

    TicketStatus getStatus() const;

    static std::string statusToString(const TicketStatus& status);

private:

    virtual bool waitCheck() const;

    TicketStatus status;

    mutable std::mutex statusMutex;
    mutable std::condition_variable statusCondition;
};


} //Engine
} //STI

#endif

