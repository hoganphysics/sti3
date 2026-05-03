#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>
#include <map>


class TestDevice : public STI::Device::LocalDevice
{
public:

	TestDevice(const STI::Utils::Configuration& config);

	//implementation of LocalDevice
	void parseEvents(const STI::Engine::RawEventMap& eventsIn, STI::Engine::SynchronousEventVector& synchedEvents);


	//Custom event class for this device's output channels
	class TestDeviceOutputEvent : public STI::Engine::SynchronousEvent
	{
	public:
		TestDeviceOutputEvent(double time);

		//implementation of SynchronousEvent pure virtual interface
		void loadEvent();
		void playEvent();
		void collectMeasurementData();
		void stopEvent();
		void pauseEvent() {}
		void unpauseEvent(bool retrigger) {}

		void addValue(short channel, const STI::Utils::MixedValue& value);

	private:

		std::map<short, STI::Utils::MixedValue> values;
	};

	//Custom event class for this device's input channel
	class TestDeviceInputEvent : public STI::Engine::SynchronousEventAdapter	//using SynchronousEventAdapter here for convenience
	{
	public:
		TestDeviceInputEvent(double time, TestDevice* device, unsigned fileIndex);

		//implementation of SynchronousEventAdapter (override only what is needed)
		void collectMeasurementData();	//only need to override collectMeasurementData() in this example 

		double exampleParameter;	//example event data...

	private:

		TestDevice* localDevice;
		unsigned fileIndex;
	};

};


#endif
