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

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

};


} //Python
} //STI

#endif

