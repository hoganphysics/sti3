#ifndef STI_ENGINE_EVENTENGINEJOB_H
#define STI_ENGINE_EVENTENGINEJOB_H

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineJobStatus.h>

#include <set>
#include <vector>


namespace STI
{
namespace Engine
{

class EventEngineDependencyTree;
class EventEngine;
class EngineID;
class Shot;
class EngineJobID;
class EngineParsingMessage;
enum class ParsingMessageType;


class EventEngineJob
{
public:

    virtual ~EventEngineJob() {}

    virtual EngineJobID getJobID() const = 0;
    virtual STI::Device::DeviceID getJobOwner() const = 0;
    virtual EngineJobStatus getStatus() const = 0;

    virtual void markRunning(const EngineID& id) = 0;
    virtual void markComplete() = 0;
    virtual void markCancelled() = 0;
    virtual void markArchived() = 0;

    virtual void attachSubjob(const std::shared_ptr<EventEngineJob>& job) = 0;

    virtual const EngineID& getEngineID() const = 0;
    virtual bool getEngine(std::shared_ptr<EventEngine>& eventEngine) const  = 0;
    virtual void setEventEngine(const std::shared_ptr<EventEngine>& eventEngine)  = 0;

    virtual bool getShot(std::shared_ptr<Shot>& shot) const = 0;
    virtual bool getDependencies(std::shared_ptr<EventEngineDependencyTree>& tree) const = 0;

    virtual std::set<STI::Device::DeviceID> getMissingTargetIDs() const = 0;

    virtual void addMessages(const std::vector<EngineParsingMessage>& messages) = 0;
    virtual EngineParsingMessage& addMessage(const ParsingMessageType& type, unsigned id, const std::string& name) = 0;
    virtual const std::vector<EngineParsingMessage>& getParsingMessages() const = 0;

};


} //Engine
} //STI

#endif
