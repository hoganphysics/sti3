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


class EventEngineFactory
{
public:

    virtual std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const EngineID& engineID, DeviceEventParser* deviceParser) = 0;
};


} //Engine
} //STI

#endif
