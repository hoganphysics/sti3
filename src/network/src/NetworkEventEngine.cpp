
#include "NetworkEventEngine.h"
#include "ORBManager.h"

using STI::Network::NetworkEventEngine;


NetworkEventEngine::NetworkEventEngine(
    const STI::Engine::EngineID& engineID,
    const STI::Device::DeviceID& localID,
    const std::shared_ptr<STI::Device::ChannelManager>& channels,
    const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
    STI::Engine::DeviceEventParser* deviceParser,
    STI::Engine::EngineTriggerTarget* triggerTarget,
    const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
    const std::shared_ptr<STI::Device::DeviceCollection>& collection,
    const std::shared_ptr<STI::Device::PersistenceManager>& persistence)
: LocalEventEngine(engineID, localID, channels, attributeManager, deviceParser, triggerTarget, dispatcher, collection, persistence), 
eventEngineServant(this) 
{
    STI::Network::ORBManager::ORBManager::activateServant(eventEngineServant);
}

NetworkEventEngine::~NetworkEventEngine()
{
}


bool NetworkEventEngine::getTEventEngineReference(const typename std::shared_ptr<STI::Engine::EventEngine>& engine, 
    STI::TNetwork::TEventEngine_var& tEngine)
{
    if (engine == 0) {
        return false;
    }
    
    auto wrapper = std::dynamic_pointer_cast<NetworkEventEngine>(engine);
    if (wrapper) {
        tEngine = wrapper->eventEngineServant._this();
        return !CORBA::is_nil(tEngine);
    }
    return false;
}

