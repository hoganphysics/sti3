#ifndef STI_TNETWORK_TDEVICE_I_H
#define STI_TNETWORK_TDEVICE_I_H

#include "deviceNet.h"

#include "Device.h"
#include "TDeviceCollection_i.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TDevice_i : public POA_STI::TNetwork::TDevice
{
public:

	TDevice_i(const std::shared_ptr<STI::Device::Device>& device);

	::CORBA::Boolean refresh();
	TDeviceCollection_ptr getDeviceCollection();
	void write(::CORBA::ULong input);

private:

	TDeviceCollection_i deviceCollectionServant;		//Servant for this Device's collection.
	std::shared_ptr<STI::Device::Device> localDevice;	//All calls to servant are forwared to this reference.
};

} //TNetwork
} //STI


#endif

