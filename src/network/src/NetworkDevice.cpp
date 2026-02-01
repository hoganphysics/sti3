
#include "NetworkDevice.h"
#include "NetworkFileServer.h"
#include "NetworkResultsCollectorFactory.h"
#include "ORBManager.h"

#include <sti/utils/FileServer.h>
#include "LocalFileServer.h"

using STI::Network::NetworkDevice;


NetworkDevice::NetworkDevice(const std::shared_ptr<STI::Device::Device>& device)
: localDevice(device), deviceServantHolder(new STI::TNetwork::TDevice_i(device))
{
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
    getMessageDispatcher(dispatcher);
    std::shared_ptr<STI::Device::ChannelManager> channels;
    getChannelManager(channels);
    std::shared_ptr<STI::Device::AttributeManager> attributeManager;
    getAttributeManager(attributeManager);
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
    getCollection(deviceCollection);
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    getPersistenceManager(persistenceManager);

    auto networkFileHolderFactory = std::make_shared<STI::Network::NetworkFileHolderFactory>(getID().getID());
    persistenceManager->setFileHolderFactory(networkFileHolderFactory);

    auto resultsCollectionFactory = std::make_shared<STI::Network::NetworkResultsCollectorFactory>();
    persistenceManager->setResultsCollectorFactory(resultsCollectionFactory);
  
    auto networkFileServer = std::make_shared<STI::Network::NetworkFileServer>(getID());
    persistenceManager->setFileServer(networkFileServer);

    auto virtualFileServerFactory = std::make_shared<STI::Network::NetworkVirtualFileServerFactory>();
    persistenceManager->setVirtualFileServerFactory(virtualFileServerFactory);

    auto networkEngineFactory = std::make_shared<STI::Network::NetworkEventEngineFactory>(
            getID(), channels, attributeManager, dispatcher, deviceCollection, persistenceManager);

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    localDevice->getEngineScheduler(scheduler);
    scheduler->setEngineFactory(networkEngineFactory);

}

