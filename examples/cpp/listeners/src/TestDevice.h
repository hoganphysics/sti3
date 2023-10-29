#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>
#include <map>


class TestDevice : public STI::Device::LocalDevice
{
public:

	TestDevice(const STI::Utils::Configuration& config);

	//implementation of LocalDevice
	bool writeChannel(short channel, const STI::Utils::MixedValue& value);


private:
	
	void init();

	bool hardwareTrigger;
};

#endif

