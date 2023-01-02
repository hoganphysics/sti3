#ifndef STI_ENGINE_RESULTSTICKET_H
#define STI_ENGINE_RESULTSTICKET_H

#include <sti/engine/Ticket.h>
#include <sti/engine/ShotID.h>
#include <sti/device/Device.h>

#include <sti/fwd/Measurement_fwd.h>
#include <sti/fwd/DeviceID_fwd.h>

#include <sti/utils/CachedValue.h>

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

    ShotID getShotID() const;
    //bool getShotRepository(std::shared_ptr<ShotRepository>& repo);

    std::shared_ptr<ParseResult> getParseResult();
    std::shared_ptr<ShotResult> getShotResult();

    STI::Engine::MeasurementVector measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);
    STI::Engine::MeasurementVector measurements(const std::string& id);

private:

    virtual bool waitCheck() const { return true; }

    STI::Engine::ShotID sid;
    // std::shared_ptr<STI::Device::Device> server;

    // bool loadResultsFromURL();

    // void loadMeasurements();

    bool isQueryable();
    bool ensureCachedMeasurements();
    bool ensureCachedParseResult();
    bool ensureCachedShotResult();


    // std::string url;
    // bool hasURL;

    // bool measurements_loaded;
    // std::shared_ptr<MeasurementVector> measurements_;

    STI::Utils::CachedValue<std::shared_ptr<MeasurementVector>> cachedMeasurements;
    STI::Utils::CachedValue<std::shared_ptr<ParseResult>> parseResult;
    STI::Utils::CachedValue<std::shared_ptr<ShotResult>> shotResult;
    
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};


} //Engine
} //STI

#endif

