
#include "NetworkDevice.h"
#include "NetworkFileServer.h"
#include "NetworkResultsCollectorFactory.h"
#include "ORBManager.h"

using STI::Network::NetworkDevice;


NetworkDevice::NetworkDevice(const std::shared_ptr<STI::Device::Device>& device)
: localDevice(device), deviceServant(device) 
{
    STI::Network::ORBManager::ORBManager::activateServant(deviceServant);

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

    auto fileServer = std::make_shared<STI::Network::NetworkFileServer>(getID());
    persistenceManager->setFileServer(fileServer);

    // std::shared_ptr<STI::Engine::ShotRepository> shotRepo;
    // persistenceManager->getShotRepository(shotRepo);
    // auto networkShotRepository = std::make_shared<STI::Network::NetworkShotRepositoryWrapper>(shotRepo);
    // persistenceManager->setShotRepository(shotRepo);

    auto networkEngineFactory = std::make_shared<STI::Network::NetworkEventEngineFactory>(
            getID(), channels, attributeManager, dispatcher, deviceCollection, persistenceManager);
    //localDevice->setEngineFactory(networkEngineFactory);

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    localDevice->getEngineScheduler(scheduler);
    scheduler->setEngineFactory(networkEngineFactory);

}

