
#include "NetworkDeviceWrapper.h"
#include "ORBManager.h"

using STI::Network::NetworkDeviceWrapper;


NetworkDeviceWrapper::NetworkDeviceWrapper(const std::shared_ptr<STI::Device::Device>& device)
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

    auto networkFileHolderFactory = std::make_shared<STI::Network::NetworkFileHolderFactory>();
    persistenceManager->setFileHolderFactory(networkFileHolderFactory);

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

