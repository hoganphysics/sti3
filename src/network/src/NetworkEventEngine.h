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
		const STI::Device::DeviceID& localID,
		STI::Device::ChannelMap& channels,
		STI::Engine::DeviceEventParser* deviceParser,
		const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher,
		const std::shared_ptr<STI::Device::DeviceCollection>& collection)
		: LocalEventEngine(localID, channels, deviceParser, dispatcher, collection), eventEngineServant(this) {}

	~NetworkEventEngine() {}


    static bool getTEventEngineReference(
        const typename std::shared_ptr<STI::Engine::EventEngine>& engine, 
        STI::TNetwork::TEventEngine_ptr& tEngine)
    {
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



} //Engine
} //STI

#endif
