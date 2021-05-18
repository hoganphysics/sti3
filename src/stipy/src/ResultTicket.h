


#ifndef STI_PYTHON_RESULTSTICKET_H
#define STI_PYTHON_RESULTSTICKET_H

#include "Ticket.h"
#include "ShotID.h"
#include "Device.h"

#include "fwd/Measurement_fwd.h"
#include "fwd/DeviceID_fwd.h"


namespace STI
{
namespace Python
{


class ResultTicket : public Ticket
{
public:

    ResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server);
    ResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus);

    STI::Engine::MeasurementVector measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);

private:

    STI::Engine::ShotID sid;
    std::shared_ptr<STI::Device::Device> server;
};


} //Python
} //STI

#endif

