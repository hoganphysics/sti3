#ifndef STI_DEVICE_JLOCALDEVICE_H
#define STI_DEVICE_JLOCALDEVICE_H

#include "JDevice.h"
#include <sti/LocalDevice.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/engine/RawEvent.h>

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

	virtual void parseEvents(const STI::Engine::RawEventMap& events, std::vector<std::shared_ptr<STI::Engine::SynchronousEventAdapter>>& synchedEvents) {}

	virtual bool writeChannel(int channel, const STI::Utils::MixedValue& value);
	virtual STI::Utils::MixedValue readChannel(int channel, const STI::Utils::MixedValue& value);
	
	bool write(int channel, const STI::Utils::MixedValue& value);
	STI::Utils::MixedValue read(int channel, const STI::Utils::MixedValue& value);
	void stopRW();

	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue);
	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues);
	LocalAttribute& addAttribute(const std::string& key, const std::string& initialValue, const std::string& allowedValues);

	LocalChannel& addChannel(int channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);

	void addEventEngine(const STI::Engine::EngineID& engineID);

	void addPartner(const DeviceID& id);
	void addPartner(const DeviceID& id, const std::string& alias);
	void addEventTarget(const DeviceID& id);
	void addEventTarget(const DeviceID& id, const std::string& alias);

	void addTask(const std::shared_ptr<STI::Utils::Task>& task);

	void sendMessage(const std::shared_ptr<DeviceMessage>& mess);

	void addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener);

	std::shared_ptr<STI::Utils::FileServer> getFileServer();

private:

	class LocalDeviceProxy : public STI::Device::LocalDevice
	{
	public:
		LocalDeviceProxy(JLocalDevice* jLocalDevice, const std::string& name, const std::string& address, unsigned short module,
			const std::string& targetServer) 
			: LocalDevice(name, address, module, targetServer), jLocalDevice(jLocalDevice) {}
		
		void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

		bool writeChannel(short channel, const STI::Utils::MixedValue& value);
		bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

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
