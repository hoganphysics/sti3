#ifndef STI_DEVICE_JLOCALDEVICE_H
#define STI_DEVICE_JLOCALDEVICE_H

#include "JDevice.h"

#include <memory>
#include <string>

namespace STI
{
namespace Device
{

class LocalDevice;
class JDeviceEventReceiver;


class JLocalDevice : public STI::Device::JDevice
{
public:
	
	JLocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JLocalDevice();

	std::shared_ptr<STI::Device::JDeviceEventReceiver> getEventReceiver();

private:

    std::shared_ptr<JDeviceEventReceiver> jReceiver;
    std::shared_ptr<LocalDevice> wrappedLocalDevice;

};

} //Device
} //STI

#endif
