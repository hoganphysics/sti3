#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>


#include <string>

class TestDevice : public STI::Device::LocalDevice
{
public:
	
	TestDevice(const STI::Utils::Configuration& config);
    TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	~TestDevice();
	
	void init();

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

	int tmp;
	
private:

	bool setTest(const std::string& value);
	std::string getTest();

	STI::Device::DeviceMessageListenerID collectionMessageLID;

	STI::Device::DeviceMessageListenerID jobMessageLID;
	STI::Device::DeviceMessageListenerID stateMessageLID;


    //Custom device event class
	class TestEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:

		TestEvent(const STI::Engine::RawEvent& evt, TestDevice* dev);
		
		void playEvent();
		void collectMeasurementData();

    private:
		STI::Engine::RawEvent evt;
		TestDevice* localDevice;
	};


};


#endif
