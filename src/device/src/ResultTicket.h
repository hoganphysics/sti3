
#ifndef STI_ENGINE_RESULTSTICKET_H
#define STI_ENGINE_RESULTSTICKET_H

#include "Ticket.h"
#include "ShotID.h"
#include "Device.h"

#include "fwd/Measurement_fwd.h"
#include "fwd/DeviceID_fwd.h"


namespace STI
{
namespace Engine
{


class ResultTicket : public Ticket
{
public:

    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server);
    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus);
    
    virtual ~ResultTicket() {}

    STI::Engine::MeasurementVector measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);

private:

    virtual bool waitCheck() { return true; }

    STI::Engine::ShotID sid;
    // std::shared_ptr<STI::Device::Device> server;

    bool loadResultsFromURL();


    std::string url;
    bool hasURL;

    bool measurements_loaded;
    std::shared_ptr<MeasurementVector> measurements_;

};


} //Engine
} //STI

#endif

