#ifndef STI_TNETWORK_TDEVICE_I_H
#define STI_TNETWORK_TDEVICE_I_H

#include "deviceNet.h"

#include "Device.h"
#include "TDeviceCollection_i.h"
#include "TDeviceEventDispatcher_i.h"

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
	TDeviceEventDispatcher_ptr getEventDispatcher();
	void write(::CORBA::ULong input);

private:

	TDeviceCollection_i deviceCollectionServant;		//Servant for this Device's collection.
	TDeviceEventDispatcher_i eventDispatcherServant;	//Servant for this Device's event dispatcher.
	std::shared_ptr<STI::Device::Device> localDevice;	//All calls to servant are forwared to this reference.
};

} //TNetwork
} //STI


#endif

