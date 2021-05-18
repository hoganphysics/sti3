#ifndef STI_DEVICE_JLOCALDEVICE_H
#define STI_DEVICE_JLOCALDEVICE_H

#include "JDevice.h"
#include "LocalDevice.h"
#include "SynchronousEvent.h"
#include "RawEvent.h"

#include <memory>
#include <string>

namespace STI
{
namespace Device
{

class LocalDevice;
class JDeviceMessageReceiver;
// class JEventEngineScheduler;
class JLocalDevice;

class JLocalDevice : public STI::Device::JDevice
{
public:
	
	JLocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~JLocalDevice();

	std::shared_ptr<STI::Device::JDeviceMessageReceiver> getMessageReceiver();
	// std::shared_ptr<STI::Engine::JEventEngineScheduler> getEngineScheduler();

//	std::shared_ptr<STI::Device::JDeviceMessageReceiver> getEventReceiver2();

//	virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) = 0;
	virtual void parseEvents(int temp) = 0;

	LocalChannel& addChannel(int channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);

	void addEventEngine(const STI::Engine::EngineID& engineID);

	void addPartner(const DeviceID& id);
//	void test();
private:

	class LocalDeviceProxy : public STI::Device::LocalDevice
	{
	public:
		LocalDeviceProxy(JLocalDevice* jLocalDevice, const std::string& name, const std::string& address, unsigned short module,
			const std::string& targetServer) 
			: LocalDevice(name, address, module, targetServer), jLocalDevice(jLocalDevice) {}
		
		void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

		class TestEvent;

		class TestEvent : public STI::Engine::SynchronousEventAdapter
		{
		public:

			TestEvent(const STI::Engine::RawEvent& evt);
			
			void playEvent();

			STI::Engine::RawEvent evt;
		};
	
	private:
		JLocalDevice* jLocalDevice;

	};

    std::shared_ptr<JDeviceMessageReceiver> jReceiver;
	// std::shared_ptr<STI::Engine::JEventEngineScheduler> jScheduler;

    std::shared_ptr<LocalDevice> wrappedLocalDevice;

};

} //Device
} //STI

#endif
