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
                                const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
                                const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection,
                                const std::shared_ptr<STI::Device::PersistenceManager>& persistence) 
    : localDeviceID(localID), channelManager(channelManager), attributeManager(attributeManager), messageDispatcher(dispatcher), 
    localCollection(collection), localPersistenceManager(persistence)
    {
    }

    std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const EngineID& engineID, DeviceEventParser* deviceParser)
    {
        auto engine = std::make_shared<STI::Engine::LocalEventEngine>(engineID, localDeviceID, channelManager, attributeManager,
        deviceParser, messageDispatcher, localCollection, localPersistenceManager);
        return engine;       
    }

private:

    STI::Device::DeviceID localDeviceID;
    std::shared_ptr<STI::Device::ChannelManager> channelManager;
    std::shared_ptr<STI::Device::AttributeManager> attributeManager;
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    std::shared_ptr<STI::Device::PersistenceManager> localPersistenceManager;

};

} //Engine
} //STI

#endif
