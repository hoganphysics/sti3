
#ifndef STI_ENGINE_PARSETICKET_H
#define STI_ENGINE_PARSETICKET_H


#include <sti/engine/Ticket.h>
#include <sti/engine/ParseID.h>
#include <sti/device/Device.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/EngineParsingMessage.h>


namespace STI
{
namespace Engine
{

class ParseResult;
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
    std::shared_ptr<RawEventGroup> getEvents();
    void getTree();
    void getTimingFiles();

private:

    virtual bool waitCheck() { return true; }

    bool parseResultBuffered;

    bool checkParseResultBuffered() const;   
    std::shared_ptr<ParseResult> parseResult;

    bool getParseResult();

    
    // bool eventsBuffered;
    // STI::Engine::DeviceEventMap events;

    // std::vector<STI::Engine::EngineParsingMessage> messages;
    STI::Engine::ParseID pid;
//    std::shared_ptr<STI::Device::Device> server;
    std::shared_ptr<EventEngineScheduler> engineScheduler;



};


} //Engine
} //STI

#endif

