#ifndef STI_DEVICE_JEVENTENGINEJOB_H
#define STI_DEVICE_JEVENTENGINEJOB_H


#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EngineJobStatus.h>
#include "DeviceIDIndexedGraph.h"

#include <memory>


namespace STI
{
namespace Engine
{

class JShot;
class JEventEngine;

class JEventEngineJob
{
public:
	
    JEventEngineJob();
	JEventEngineJob(const std::shared_ptr<EventEngineJob>& eventEngineJob);
    ~JEventEngineJob();

    EngineJobID getJobID() const;
    STI::Device::DeviceID getJobOwner() const;
    EngineJobStatus getStatus() const;

    EngineID getEngineID() const;
    std::shared_ptr<JEventEngine> getEngine() const;

    std::shared_ptr<JShot> getShot() const;
    std::shared_ptr<EventEngineDependencyTree> getDependencies() const;
    
    STI::Device::DeviceIDIndexedGraph getDependenciesIndexed() const;

    std::set<STI::Device::DeviceID> getMissingTargetIDs() const;

    std::vector<EngineParsingMessage> getParsingMessages() const;

private:

    friend class JEventEngine;

    std::shared_ptr<EventEngineJob> eventEngineJob;
    std::shared_ptr<JShot> jshot;

};


} //Engine
} //STI

#endif
