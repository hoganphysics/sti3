#ifndef STI_NETWORK_TDEVICEREFINTERFACE_H
#define STI_NETWORK_TDEVICEREFINTERFACE_H

#include <sti/device/Device.h>
#include "generated/orbTypes.h"

#include <memory>

namespace STI
{
namespace Network
{


//Abstract interface for extracting TDevice references from appropriate Device pointers
class TDeviceRefInterface
{
public:

	static bool getTDeviceReference(const typename std::shared_ptr<STI::Device::Device>& device, STI::TNetwork::TDevice_var& tDevice)
	{
		std::shared_ptr<TDeviceRefInterface> tDeviceRefInterface;
		tDeviceRefInterface = std::dynamic_pointer_cast<TDeviceRefInterface>(device);

		return (tDeviceRefInterface != 0 &&					//check dynamic_pointer_cast
			tDeviceRefInterface->getTDeviceRef(tDevice) &&	//polymorphic call
			!CORBA::is_nil(tDevice)
			);
		
		return true;
	}


	// static bool getTDeviceReference(const typename std::shared_ptr<STI::Device::Device>& device, STI::TNetwork::TDevice_ptr& tDevice)
	// {
	// 	std::shared_ptr<TDeviceRefInterface> tDeviceRefInterface;
	// 	tDeviceRefInterface = std::dynamic_pointer_cast<TDeviceRefInterface>(device);

	// 	return (tDeviceRefInterface != 0 &&					//check dynamic_pointer_cast
	// 		tDeviceRefInterface->getTDeviceRef(tDevice) &&	//polymorphic call
	// 		!CORBA::is_nil(tDevice)
	// 		);
	// }

private:

//	virtual bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice) = 0;

	virtual bool getTDeviceRef(STI::TNetwork::TDevice_var& tDevice) = 0;

};


} //Network
} //STI


#endif

