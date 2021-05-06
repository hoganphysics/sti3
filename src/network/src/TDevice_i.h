#ifndef STI_TNETWORK_TDEVICE_I_H
#define STI_TNETWORK_TDEVICE_I_H

#include "deviceNet.h"

#include "Device.h"
#include "TDeviceCollection_i.h"
#include "TDeviceMessageDispatcher_i.h"
#include "TEventEngineScheduler_i.h"
#include "TChannelManager_i.h"
#include "TAttributeManager_i.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TDevice_i : public POA_STI::TNetwork::TDevice
{
public:

	TDevice_i(const std::shared_ptr<STI::Device::Device>& device);
	~TDevice_i();

	::CORBA::Boolean refresh();
	TDeviceCollection_ptr getDeviceCollection();
	TDeviceMessageDispatcher_ptr getMessageDispatcher();
	TEventEngineScheduler_ptr getEngineScheduler();
	TChannelManager_ptr getChannelManager();
	TAttributeManager_ptr getAttributeManager();
	TDeviceID* getID();


private:

	TDeviceCollection_i deviceCollectionServant;		//Servant for this Device's collection.
	TDeviceMessageDispatcher_i messageDispatcherServant;	//Servant for this Device's event dispatcher.
	TEventEngineScheduler_i eventSchedulerServant;		//Servant for this Device's event scheduler.
	TChannelManager_i channelManagerServant;
	TAttributeManager_i attributeManagerServant;

	std::shared_ptr<STI::Device::Device> localDevice;	//All calls to servant are forwared to this reference.
};

} //TNetwork
} //STI


#endif

