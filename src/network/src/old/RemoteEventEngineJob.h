#ifndef STI_NETWORK_REMOTEEVENTENGINEJOB_H
#define STI_NETWORK_REMOTEEVENTENGINEJOB_H

#include "deviceNet.h"

#include "EventEngineJob.h"


#include <memory>

namespace STI
{
namespace Network
{

class RemoteEventEngineJob : public STI::Engine::EventEngineJob
{
public:

	RemoteEventEngineJob(::STI::TNetwork::TEventEngineJob_ptr engineJob);
    ~RemoteEventEngineJob();
    
    STI::Engine::EngineJobID getJobID() const;
    STI::Device::DeviceID getJobOwner() const;
    EngineJobStatus getStatus() const;

    void markRunning(const  STI::Engine::EngineID& id);
    void markComplete();
    void markCancelled();

    const  STI::Engine::EngineID& getEngineID() const;
    bool getEngine(std::shared_ptr< STI::Engine::EventEngine>& eventEngine) const ;
    void setEventEngine(const std::shared_ptr< STI::Engine::EventEngine>& eventEngine) ;

    bool getParsedShot(std::shared_ptr< STI::Engine::ParsedShot>& shot) const;
    bool getDependencies(std::shared_ptr< STI::Engine::EventEngineDependencyTree>& tree) const;

    std::set<STI::Device::DeviceID> getMissingTargetIDs() const;

private:

	::STI::TNetwork::TEventEngineJob_var tEventEngineJob;		//remote reference

};


} //Network
} //STI


#endif

