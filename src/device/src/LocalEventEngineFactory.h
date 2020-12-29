#ifndef STI_ENGINE_LOCALEVENTENGINEFACTORY_H
#define STI_ENGINE_LOCALEVENTENGINEFACTORY_H

#include "EventEngineFactory.h"

#include <memory>


namespace STI
{

namespace Engine
{

class LocalEventEngineFactory : public EventEngineFactory
{
public:

    std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const STI::Device::DeviceID& localID, 
                                const std::shared_ptr<STI::Device::ChannelManager>& channels,
                                DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection)
    {
        auto engine = std::make_shared<STI::Engine::LocalEventEngine>(localID, channels, deviceParser, dispatcher, collection);
        return engine;
    }
};

} //Engine
} //STI

#endif
