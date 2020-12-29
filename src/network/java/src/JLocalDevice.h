#ifndef STI_DEVICE_JLOCALDEVICE_H
#define STI_DEVICE_JLOCALDEVICE_H

#include "JDevice.h"
#include "LocalDevice.h"

#include <memory>
#include <string>

namespace STI
{
namespace Device
{

class LocalDevice;
class JDeviceEventReceiver;
class JEventEngineScheduler;
class JLocalDevice;

class JLocalDevice : public STI::Device::JDevice
{
public:
	
	JLocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JLocalDevice();

	std::shared_ptr<STI::Device::JDeviceEventReceiver> getEventReceiver();
	std::shared_ptr<STI::Device::JEventEngineScheduler> getEngineScheduler();

//	virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) = 0;
	virtual void parseEvents(int temp) = 0;

private:

	class LocalDeviceProxy : public STI::Device::LocalDevice
	{
	public:
		LocalDeviceProxy(JLocalDevice* jLocalDevice, const std::string& name, const std::string& address, unsigned short module,
			const std::string& targetServer) 
			: LocalDevice(name, address, module, targetServer), jLocalDevice(jLocalDevice) {}
		
		void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
		{
			if (jLocalDevice != 0) {
				jLocalDevice->parseEvents(0);	//temp
			}
		}
	
	private:
		JLocalDevice* jLocalDevice;

	};

    std::shared_ptr<JDeviceEventReceiver> jReceiver;
	std::shared_ptr<JEventEngineScheduler> jScheduler;

    std::shared_ptr<LocalDevice> wrappedLocalDevice;

};

} //Device
} //STI

#endif
