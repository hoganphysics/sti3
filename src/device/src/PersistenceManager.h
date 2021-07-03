#ifndef STI_DEVICE_PERSISTENCEMANAGER_H
#define STI_DEVICE_PERSISTENCEMANAGER_H

#include "ShotID.h"
#include "ResultTicket.h"
#include "ResultsCollector.h"
#include "EventEngineJob.h"
// #include "ResultsCollectorFactory.h"
#include "FileHolderFactory.h"

#include <memory>


namespace STI
{
namespace Device
{


class PersistenceManager
{
public:

    virtual ~PersistenceManager() {}

    virtual bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine) = 0;

    virtual bool transferMeasurements(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector) = 0;

    virtual bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket) = 0;
    
	//could be part of PersistenceManager
	virtual void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;


    // virtual std::shared_ptr<STI::Engine::ResultsCollector> createResultsCollector(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::EventEngine>& eventEngine) = 0;

    // //virtual void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;
    // virtual void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) = 0;

    // virtual void saveShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) = 0;
    // virtual void copyShot(const std::shared_ptr<STI::Engine::ResultsCollector>& collector) = 0;

};


} //Device
} //STI

#endif
