
#ifndef STI_PYTHON_PARSETICKET_H
#define STI_PYTHON_PARSETICKET_H


#include "Ticket.h"
#include "ParseID.h"
#include "Device.h"
#include "fwd/RawEvent_fwd.h"
#include "EngineParsingMessage.h"

// #include <string>
// #include <mutex>
// #include <condition_variable>

namespace STI
{
namespace Python
{


class ResultTicket;
class ParseTicket;
// class ParseTicketManager;

class ParseTicket : public Ticket
{
public:

    ParseTicket(const STI::Engine::ParseID& id, 
                const std::shared_ptr<STI::Device::Device>& server);
    ~ParseTicket();
 
    const STI::Engine::ParseID& getParseID() const;

    // void wait();    //blocks until parse completes; returns this for chaining

    // // ResultTicket play();
    // // ResultTicket play(unsigned repeats);

    // void setComplete();
    // void cancel();   //cancels parse and stops wait()

    // enum class ParseTicketStatus { Parsing, Complete, Cancelled };

    std::vector<STI::Engine::EngineParsingMessage> getMessages();
    STI::Engine::DeviceEventMap& getEvents();
    void getTree();

private:

    bool eventsBuffered;
    STI::Engine::DeviceEventMap events;

    bool messagesBuffered;
    std::vector<STI::Engine::EngineParsingMessage> messages;

    STI::Engine::ParseID pid;

    // ParseTicketStatus status;

    // ParseTicketManager* ticketManager;

    std::shared_ptr<STI::Device::Device> server;

    // mutable std::mutex parseMutex;
    // mutable std::condition_variable parseCondition;

};


} //Python
} //STI

#endif

