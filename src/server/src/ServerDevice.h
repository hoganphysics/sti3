#ifndef STI_NETWORK_SERVERDEVICE_H
#define STI_NETWORK_SERVERDEVICE_H

#include <sti/LocalDevice.h>


namespace STI
{
namespace Device
{

class ServerDevice : public STI::Device::LocalDevice
{
public:
	ServerDevice(const STI::Utils::Configuration& config);
    // ServerDevice(const std::string& name, const std::string& address, unsigned short module,
	// 	const std::string& targetServer);
	~ServerDevice();

	// bool writeChannel(short channel, const STI::Utils::MixedValue& value);

	void parseEvents(const STI::Engine::RawEventMap& eventsIn, STI::Engine::SynchronousEventVector& synchedEvents);

	STI::Device::DeviceID partnerID;

	//Custom event class for this device's output channels
	class TestDeviceOutputEvent : public STI::Engine::SynchronousEvent
	{
	public:
		TestDeviceOutputEvent(double time);

		//implementation of SynchronousEvent pure virtual interface
		void loadEvent();
		void playEvent();
		void collectMeasurementData() {}
		void stopEvent() {}
		void pauseEvent() {}
		void unpauseEvent(bool retrigger) {}

		void addValue(short channel, const STI::Utils::MixedValue& value);

	private:

		std::map<short, STI::Utils::MixedValue> values;
	};

};


} //Device
} //STI


#endif

