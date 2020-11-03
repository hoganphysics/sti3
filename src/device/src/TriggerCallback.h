#ifndef STI_ENGINE_TRIGGERCALLBACK_H
#define STI_ENGINE_TRIGGERCALLBACK_H

#include "fwd/DeviceID_fwd.h"

namespace STI
{
namespace Engine
{

class TriggerCallbackTarget		//interface
{
public:

	virtual ~TriggerCallbackTarget() {}

	virtual void ready(const STI::Device::DeviceID& id) = 0;
	virtual void triggerFired(const STI::Device::DeviceID& id) = 0;

};

class TriggerCallback
{
public:

	TriggerCallback(TriggerCallbackTarget* target) : cbTarget(target) {}
	~TriggerCallback() {}

	void ready(const STI::Device::DeviceID& id) { cbTarget->ready(id); }
	void triggerFired(const STI::Device::DeviceID& id) { cbTarget->triggerFired(id); }

private:

	TriggerCallbackTarget* cbTarget;
};

} //Engine
} //STI

#endif
