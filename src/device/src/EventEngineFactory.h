#ifndef STI_ENGINE_EVENTENGINEFACTORY_H
#define STI_ENGINE_EVENTENGINEFACTORY_H

#include <memory>


namespace STI
{
namespace Engine
{

class LocalEventEngine;
class EngineID;
class DeviceEventParser;
class EngineTriggerTarget;


class EventEngineFactory
{
public:

    virtual std::shared_ptr<LocalEventEngine> createEngine(const EngineID& engineID, DeviceEventParser* deviceParser, EngineTriggerTarget* triggerTarget) = 0;
};


} //Engine
} //STI

#endif
