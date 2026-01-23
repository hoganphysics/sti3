#ifndef STI_TNETWORK_TTRIGGERCALLBACK_I_H
#define STI_TNETWORK_TTRIGGERCALLBACK_I_H

#include "generated/deviceNet.h"
#include "TriggerCallback.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TTriggerCallback_i : public POA_STI::TNetwork::TTriggerCallback,
                           public PortableServer::RefCountServantBase
{
public:

	TTriggerCallback_i(const std::shared_ptr<STI::Engine::TriggerCallback>& triggerCB);
	~TTriggerCallback_i();

    void ready(const ::STI::TNetwork::TDeviceID& id);
    void triggerFired(const ::STI::TNetwork::TDeviceID& id);

private:

    std::shared_ptr<STI::Engine::TriggerCallback> _triggerCB;

};

} //TNetwork
} //STI

#endif
