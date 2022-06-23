#ifndef STI_DEVICE_PERSISTENCEMANAGER_H
#define STI_DEVICE_PERSISTENCEMANAGER_H

#include <sti/engine/ShotID.h>
#include <sti/engine/ResultTicket.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/EventEngineJob.h>
// #include "ResultsCollectorFactory.h"
#include <sti/utils/FileHolderFactory.h>
//#include "ShotRepository.h"
#include <sti/engine/ShotResultRecord.h>

#include <memory>


namespace STI
{
namespace Device
{


class PersistenceManager
{
public:

    virtual ~PersistenceManager() {}

    virtual bool getShot(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result) = 0;
    virtual bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::ShotResult>& shotResult, bool isOwner) = 0;

    virtual STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector) = 0;

    virtual bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements) = 0;

    //virtual bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket) = 0;
    
	virtual void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;

//    virtual void setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo) = 0;
//    virtual bool getShotRepository(std::shared_ptr<STI::Engine::ShotRepository>& repo) = 0;

    // virtual std::shared_ptr<STI::Engine::ResultsCollector> createResultsCollector(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine) = 0;

    // //virtual void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;
    // virtual void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) = 0;

    // virtual void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) = 0;
    // virtual void copyShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) = 0;

};


} //Device
} //STI

#endif
