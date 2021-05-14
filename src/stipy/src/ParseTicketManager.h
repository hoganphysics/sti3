#ifndef STI_PYTHON_STIPARSETICKETMANAGER_H
#define STI_PYTHON_STIPARSETICKETMANAGER_H

#include "ParseID.h"
#include "TicketManager.h"
#include "DeviceMessageListener.h"
#include "Device.h"

#include <map>
#include <memory>
#include <mutex>

namespace STI
{
namespace Python
{



class ParseTicket;


class ParseTicketManager : public TicketManager<STI::Engine::ParseID, ParseTicket>,
                           public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    ParseTicketManager();
    ~ParseTicketManager();

    // void add(const std::shared_ptr<ParseTicket>& ticket);
    // void remove(const STI::Engine::ParseID& id);

    // void cancel(const STI::Engine::ParseID& id);
    // void cancelAll();

    // std::shared_ptr<ParseTicket> makeParseTicket(const STI::Engine::ParseID& pid, 
    //                                             const std::shared_ptr<STI::Device::Device>& server);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    // std::map<STI::Engine::ParseID, std::shared_ptr<ParseTicket>> tickets;

    // mutable std::mutex ticketMutex;
};




} //Python
} //STI

#endif

