#ifndef STI_ENGINE_RESULTSTICKET_H
#define STI_ENGINE_RESULTSTICKET_H

#include <sti/engine/Ticket.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/device/Device.h>

#include <sti/fwd/Measurement_fwd.h>
#include <sti/fwd/DeviceID_fwd.h>

#include <sti/utils/CachedValue.h>

#include <vector>

namespace STI
{
namespace Engine
{

class ShotResult;
class ParseResult;


class ResultTicket : public Ticket  //, public ShotResult
{
public:

    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager);
    ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, const TicketStatus& initialStatus);

    virtual ~ResultTicket() {}

    ShotID getShotID() const;

    std::shared_ptr<ParseResult> getParseResult();
    std::shared_ptr<ShotResult> getShotResult();

    STI::Engine::MeasurementMap measurements();
    STI::Engine::MeasurementVector measurements(const STI::Device::DeviceID& id);
    STI::Engine::MeasurementVector measurements(const std::string& id);

    std::vector<EnginePlayingMessage> getMessages();

    bool getMeasurements(std::shared_ptr<STI::Engine::MeasurementMap>& measurements);

private:

    virtual bool waitCheck() const { return true; }

    STI::Engine::ShotID sid;

    bool isQueryable();
    bool ensureCachedMeasurements();
    bool ensureCachedParseResult();
    bool ensureCachedShotResult();

    STI::Utils::CachedValue<std::shared_ptr<MeasurementMap>> cachedMeasurements;
    STI::Utils::CachedValue<std::shared_ptr<ParseResult>> parseResult;
    STI::Utils::CachedValue<std::shared_ptr<ShotResult>> shotResult;
    
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
};


} //Engine
} //STI

#endif

