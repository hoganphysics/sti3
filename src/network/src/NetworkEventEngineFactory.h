#ifndef STI_NETWORK_NETWORKEVENTENGINEFACTORY_H
#define STI_NETWORK_NETWORKEVENTENGINEFACTORY_H

#include "EventEngineFactory.h"
#include "NetworkEventEngine.h"

#include <memory>

namespace STI
{

namespace Network
{

class NetworkEventEngineFactory : public STI::Engine::EventEngineFactory
{

public:

    NetworkEventEngineFactory(const STI::Device::DeviceID& localID, 
                                const std::shared_ptr<STI::Device::ChannelManager>& channelManager,
                                const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection,
                                const std::shared_ptr<STI::Device::PersistenceManager>& persistence) 
    : localDeviceID(localID), channelManager(channelManager), messageDispatcher(dispatcher), 
        localCollection(collection), persistenceManager(persistence)
    {
    }

    std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const STI::Engine::EngineID& engineID, STI::Engine::DeviceEventParser* deviceParser)
    {
        auto networkEngine = std::make_shared<NetworkEventEngine>(engineID,
                                            localDeviceID, channelManager, deviceParser, 
                                            messageDispatcher, localCollection, persistenceManager);
        return networkEngine;
    }


    // std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const STI::Device::DeviceID& localID, 
    //                             const std::shared_ptr<STI::Device::ChannelManager>& channels, 
    //                             DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
    //                             const std::shared_ptr<STI::Device::DeviceCollection>& collection)
    // {
    //     //auto localEngine = std::make_shared<STI::Engine::LocalEventEngine>(localID, channels, deviceParser, dispatcher, collection);

    //     //auto networkEngine = std::make_shared<STI::Network::NetworkEventEngine>(localID, channels, deviceParser, dispatcher, collection);
    //     auto networkEngine = std::make_shared<STI::Network::NetworkEventEngine>(localID, channels, deviceParser, dispatcher, collection);
    //     return networkEngine;
    // }

private:

    STI::Device::DeviceID localDeviceID;
    std::shared_ptr<STI::Device::ChannelManager> channelManager;
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
};

} //Network
} //STI

#endif
