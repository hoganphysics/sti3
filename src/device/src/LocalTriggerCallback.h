#ifndef STI_ENGINE_LOCALTRIGGERCALLBACK_H
#define STI_ENGINE_LOCALTRIGGERCALLBACK_H

#include "TriggerCallback.h"


namespace STI
{
namespace Engine
{


class LocalTriggerCallback : public TriggerCallback
{
public:

	LocalTriggerCallback(TriggerCallbackTarget* target) : cbTarget(target) {}
	~LocalTriggerCallback() {}

	void ready(const STI::Device::DeviceID& id) { cbTarget->ready(id); }
	void triggerFired(const STI::Device::DeviceID& id) { cbTarget->triggerFired(id); }

private:

	TriggerCallbackTarget* cbTarget;
};

} //Engine
} //STI

#endif
