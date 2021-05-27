#ifndef STI_NETWORK_REMOTETRIGGERCALLBACK_H
#define STI_NETWORK_REMOTETRIGGERCALLBACK_H

#include "deviceNet.h"

#include "TriggerCallback.h"
#include "TReferenceHolder.h"


namespace STI
{
namespace Network
{

class RemoteTriggerCallback : public STI::Engine::TriggerCallback,
							  public STI::TNetwork::TReferenceHolder<STI::TNetwork::TTriggerCallback>	//mixin
{
public:

	RemoteTriggerCallback(::STI::TNetwork::TTriggerCallback_ptr trigger);
    ~RemoteTriggerCallback();

	void ready(const STI::Device::DeviceID& id);
	void triggerFired(const STI::Device::DeviceID& id);

private:

//    ::STI::TNetwork::TTriggerCallback_var _tTrigger; //remote reference
	mutable std::mutex cbMutex;
};


} //Network
} //STI


#endif

