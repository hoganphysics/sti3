#ifndef STI_DEVICE_PERSISTENCEMANAGER_H
#define STI_DEVICE_PERSISTENCEMANAGER_H

#include <sti/engine/EventEngineJob.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/ResultTicket.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/utils/FileHolderFactory.h>

#include <memory>


namespace STI
{
namespace Device
{


class PersistenceManager : public STI::Utils::FileHolderFactory
{
public:

    virtual ~PersistenceManager() {}

    virtual bool findShot(const STI::Engine::ShotID& sid) = 0;

    virtual bool getParseResult(const STI::Engine::ParseID& pid, std::shared_ptr<STI::Engine::ParseResult>& parseResult) = 0;
    virtual bool getShotResult(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ShotResult>& result) = 0;
    virtual bool getSequenceResult(const STI::Engine::SequenceID& id, std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult) = 0;

    virtual bool saveShot(const STI::Engine::ShotID& sid, const std::shared_ptr<STI::Engine::FullShotResult>& fullShotResult, bool isOwner) = 0;

    virtual STI::Engine::ShotResultRecord transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector) = 0;

    virtual void setResultsCollectorFactory(const std::shared_ptr<STI::Engine::ResultsCollectorFactory>& factory) = 0;

    virtual bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementMap>& measurements) = 0;

    //virtual bool getResultTicket(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::ResultTicket>& ticket) = 0;
    
	virtual void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;

    virtual void setFileServer(const std::shared_ptr<STI::Utils::FileServer>& server) = 0;
    virtual bool getFileServer(std::shared_ptr<STI::Utils::FileServer>& server) = 0;

    virtual void addSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult) = 0;
    virtual bool updateSequence(const STI::Engine::SequenceEntryID& id, const STI::Engine::ShotID& shotID, const STI::Engine::EngineJobStatus& shotStatus, bool isOwner) = 0;
    virtual bool saveSequence(const std::shared_ptr<STI::Engine::SequenceResult>& sequenceResult, bool isOwner) = 0;
    
    // virtual bool getLogDirectory(std::string& logDirectory) = 0;

    // virtual std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& filename) = 0;
    

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
