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

    LocalEventEngineFactory(const STI::Device::DeviceID& localID, 
                                const std::shared_ptr<STI::Device::ChannelManager>& channelManager,
                                const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection) 
    : localDeviceID(localID), channelManager(channelManager), messageDispatcher(dispatcher), localCollection(collection)
    {
    }

    std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const EngineID& engineID, DeviceEventParser* deviceParser)
    {
        auto engine = std::make_shared<STI::Engine::LocalEventEngine>(engineID, localDeviceID, channelManager, deviceParser, messageDispatcher, localCollection);
        return engine;       
    }

private:

    STI::Device::DeviceID localDeviceID;
    std::shared_ptr<STI::Device::ChannelManager> channelManager;
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;

};

} //Engine
} //STI

#endif
