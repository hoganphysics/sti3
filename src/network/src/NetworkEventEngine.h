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
        const std::shared_ptr<STI::Device::PersistenceManager>& persistence);

	~NetworkEventEngine();


    static bool getTEventEngineReference(
        const typename std::shared_ptr<STI::Engine::EventEngine>& engine, STI::TNetwork::TEventEngine_var& tEngine);

private:

    STI::TNetwork::TEventEngine_i eventEngineServant;
};


} //Network
} //STI

#endif
