
#ifndef STI_ENGINE_RESULTSTICKET_H
#define STI_ENGINE_RESULTSTICKET_H

#include <sti/engine/Ticket.h>
#include <sti/engine/ShotID.h>
#include <sti/device/Device.h>

#include <sti/fwd/Measurement_fwd.h>
#include <sti/fwd/DeviceID_fwd.h>


namespace STI
{
namespace Engine
{

class ShotResult;
class ParseResult;

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

    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager);
    // ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus);
    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, const TicketStatus& initialStatus);
    // ResultTicket(const STI::Engine::ShotID& id, const std::string& url);

    virtual ~ResultTicket() {}

    ShotID getShotID();
    //bool getShotRepository(std::shared_ptr<ShotRepository>& repo);

    bool getShotResult(const STI::Engine::ShotID& id, std::shared_ptr<ShotResult>& result);

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

    std::shared_ptr<ParseResult> parseResult;
    std::shared_ptr<ShotResult> shotResult;
    
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};


} //Engine
} //STI

#endif

