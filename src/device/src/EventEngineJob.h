#ifndef STI_ENGINE_EVENTENGINEJOB_H
#define STI_ENGINE_EVENTENGINEJOB_H

#include "DeviceID.h"

#include <set>

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

    enum class EngineJobStatus { New, Running, Completed, Cancelled };

    virtual ~EventEngineJob() {}

    virtual EngineJobID getJobID() const = 0;
    virtual STI::Device::DeviceID getJobOwner() const = 0;
    virtual EngineJobStatus getStatus() const = 0;

    virtual void markRunning(const EngineID& id) = 0;
    virtual void markComplete() = 0;
    virtual void markCancelled() = 0;

    virtual void attachSubjob(const std::shared_ptr<EventEngineJob>& job) = 0;

    virtual const EngineID& getEngineID() const = 0;
    virtual bool getEngine(std::shared_ptr<EventEngine>& eventEngine) const  = 0;
    virtual void setEventEngine(const std::shared_ptr<EventEngine>& eventEngine)  = 0;

    virtual bool getParsedShot(std::shared_ptr<Shot>& shot) const = 0;
    virtual bool getDependencies(std::shared_ptr<EventEngineDependencyTree>& tree) const = 0;

    virtual std::set<STI::Device::DeviceID> getMissingTargetIDs() const = 0;

    virtual EngineParsingMessage& addMessage(const ParsingMessageType& type, unsigned id, const std::string& name) = 0;

};


} //Engine
} //STI

#endif
