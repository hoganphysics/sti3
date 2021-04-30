
#ifndef STI_PYTHON_PARSETICKET_H
#define STI_PYTHON_PARSETICKET_H

#include "ParseID.h"

#include <string>
#include <mutex>
#include <condition_variable>

namespace STI
{
namespace Python
{


class ResultTicket;
class ParseTicket;
class ParseTicketManager;

class ParseTicket
{
public:

    ParseTicket(const STI::Engine::ParseID& id, ParseTicketManager* manager);
    ~ParseTicket();
 
    const STI::Engine::ParseID& getParseID() const;

    ParseTicket& wait();    //blocks until parse completes; returns this for chaining

    // ResultTicket play();
    // ResultTicket play(unsigned repeats);

    void setComplete();
    void cancel();   //cancels parse and stops wait()

    enum class ParseTicketStatus { Parsing, Complete, Cancelled };

private:

    STI::Engine::ParseID pid;

    ParseTicketStatus status;

    ParseTicketManager* ticketManager;

    mutable std::mutex parseMutex;
    mutable std::condition_variable parseCondition;

};


} //Python
} //STI

#endif

