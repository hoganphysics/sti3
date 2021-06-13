#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include "LocalDevice.h"
#include "SynchronousEvent.h"
#include "RawEvent.h"

#include <string>

class TestDevice : public STI::Device::LocalDevice
{
public:
	
    TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	~TestDevice();

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

	STI::Device::DeviceMessageListenerID collectionMessageLID;

    //Custom device event class
	class TestEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:

		TestEvent(const STI::Engine::RawEvent& evt, TestDevice* dev);
		
		void playEvent();

    private:
		STI::Engine::RawEvent evt;
		STI::Device::LocalDevice* localDevice;
	};
};


#endif
