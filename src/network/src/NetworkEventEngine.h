#ifndef STI_ENGINE_NETWORKEVENTENGINE_H
#define STI_ENGINE_NETWORKEVENTENGINE_H

#include "LocalEventEngine.h"
#include "TEventEngine_i.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{

class NetworkEventEngine : public STI::Engine::LocalEventEngine
{
public:

	NetworkEventEngine(
        const STI::Engine::EngineID& engineID,
		const STI::Device::DeviceID& localID,
		const std::shared_ptr<STI::Device::ChannelManager>& channels,
        const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
		STI::Engine::DeviceEventParser* deviceParser,
		const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
		const std::shared_ptr<STI::Device::DeviceCollection>& collection,
        const std::shared_ptr<STI::Device::PersistenceManager>& persistence)
		: LocalEventEngine(engineID, localID, channels, attributeManager, deviceParser, dispatcher, collection, persistence), eventEngineServant(this) {}

	~NetworkEventEngine() {}


    static bool getTEventEngineReference(
        const typename std::shared_ptr<STI::Engine::EventEngine>& engine, 
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

private:

    STI::TNetwork::TEventEngine_i eventEngineServant;
};



} //Network
} //STI

#endif
