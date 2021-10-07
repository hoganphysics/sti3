
#ifndef STI_ENGINE_PARSETICKET_H
#define STI_ENGINE_PARSETICKET_H


#include "Ticket.h"
#include "ParseID.h"
#include "Device.h"
#include "fwd/RawEvent_fwd.h"
#include "EngineParsingMessage.h"


namespace STI
{
namespace Engine
{


class ResultTicket;
class ParseTicket;


class ParseTicket : public Ticket
{
public:

    ParseTicket(const STI::Engine::ParseID& id, 
                const std::shared_ptr<EventEngineScheduler>& scheduler);
    virtual ~ParseTicket();
 
    const STI::Engine::ParseID& getParseID() const;

    std::vector<STI::Engine::EngineParsingMessage> getMessages();
    STI::Engine::DeviceEventMap& getEvents();
    void getTree();
    void getTimingFiles();

private:

    virtual bool waitCheck() { return true; }

    bool eventsBuffered;
    STI::Engine::DeviceEventMap events;

    bool messagesBuffered;
    std::vector<STI::Engine::EngineParsingMessage> messages;
    STI::Engine::ParseID pid;
//    std::shared_ptr<STI::Device::Device> server;
    std::shared_ptr<EventEngineScheduler> engineScheduler;

};


} //Engine
} //STI

#endif

