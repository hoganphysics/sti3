
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

class ShotRepository;

// class ShotResult
// {
// public:

//     virtual ~ShotResult() {}

//     virtual STI::Engine::MeasurementVector measurements() = 0;
//     virtual STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id) = 0;
// };


	

//class URLShotResult : public ShotResult

class ResultTicket : public Ticket  //, public ShotResult
{
public:

    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<ShotRepository>& shotRepository);
    // ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus);
    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<ShotRepository>& repo, const TicketStatus& initialStatus);
    // ResultTicket(const STI::Engine::ShotID& id, const std::string& url);

    virtual ~ResultTicket() {}

    ShotID getShotID();
    bool getShotRepository(std::shared_ptr<ShotRepository>& repo);

    STI::Engine::MeasurementVector measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);

private:

    virtual bool waitCheck() { return true; }

    STI::Engine::ShotID sid;
    // std::shared_ptr<STI::Device::Device> server;

    bool loadResultsFromURL();

    void loadMeasurements();


    std::string url;
    bool hasURL;

    bool measurements_loaded;
    std::shared_ptr<MeasurementVector> measurements_;

    std::shared_ptr<ShotRepository> shotRepository;

};


} //Engine
} //STI

#endif

