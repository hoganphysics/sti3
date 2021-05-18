
#ifndef STI_PYTHON_RESULTTICKETMANAGER_H
#define STI_PYTHON_RESULTTICKETMANAGER_H

#include "ShotID.h"
#include "ResultTicket.h"
#include "TicketManager.h"
#include "DeviceMessageListener.h"

#include <memory>


namespace STI
{
namespace Python
{

// class ShotID;
class ResultTicket;


class ResultTicketManager : public TicketManager<STI::Engine::ShotID, ResultTicket>,
                            public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    ResultTicketManager();
    ~ResultTicketManager();

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

};


} //Python
} //STI

#endif

