#ifndef STI_ENGINE_EVENTENGINEFACTORY_H
#define STI_ENGINE_EVENTENGINEFACTORY_H

#include "Device.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "DeviceEventParser.h"
#include "DeviceEventDispatcher.h"
#include "fwd/Channel_fwd.h"
#include "fwd/ChannelManager_fwd.h"

#include <memory>


namespace STI
{

namespace Engine
{

class LocalEventEngine;


class EventEngineFactory
{
public:
    virtual std::shared_ptr<STI::Engine::LocalEventEngine> createEngine(const STI::Device::DeviceID& localID, 
                                const std::shared_ptr<STI::Device::ChannelManager>& channels, 
                                DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, 
                                const std::shared_ptr<STI::Device::DeviceCollection>& collection) = 0;
};


} //Engine
} //STI

#endif
