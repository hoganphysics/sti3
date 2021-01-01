#ifndef STI_ENGINE_NETWORKEVENTENGINEFACTORY_H
#define STI_ENGINE_NETWORKEVENTENGINEFACTORY_H

#include "EventEngineFactory.h"
#include "NetworkEventEngine.h"

#include <memory>

namespace STI
{

namespace Engine
{

class NetworkEventEngineFactory : public EventEngineFactory
{
    std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const STI::Device::DeviceID& localID, 
                                const std::shared_ptr<STI::Device::ChannelManager>& channels, 
                                DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection)
    {
        //auto localEngine = std::make_shared<STI::Engine::LocalEventEngine>(localID, channels, deviceParser, dispatcher, collection);

        //auto networkEngine = std::make_shared<STI::Network::NetworkEventEngine>(localID, channels, deviceParser, dispatcher, collection);
        auto networkEngine = std::make_shared<STI::Network::NetworkEventEngine>(localID, channels, deviceParser, dispatcher, collection);
        return networkEngine;
    }
};

} //Engine
} //STI

#endif
