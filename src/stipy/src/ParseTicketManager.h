#ifndef STI_PYTHON_STIPARSETICKETMANAGER_H
#define STI_PYTHON_STIPARSETICKETMANAGER_H

#include "ParseID.h"
#include "DeviceMessageListener.h"

#include <map>
#include <memory>

namespace STI
{
namespace Python
{

class ParseTicket;



class ParseTicketManager : public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    ParseTicketManager();
    ~ParseTicketManager();

    void add(const ParseTicket&);
    void remove(const STI::Engine::ParseID&);

    void cancel(const STI::Engine::ParseID&);
    void cancelAll();

    std::shared_ptr<ParseTicket> makeParseTicket(const STI::Engine::ParseID& pid);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    std::map<STI::Engine::ParseID, std::shared_ptr<ParseTicket>> tickets;
};




} //Python
} //STI

#endif

