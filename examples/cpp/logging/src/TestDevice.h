#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>


class TestDevice : public STI::Device::LocalDevice
{
public:

	TestDevice(const STI::Utils::Configuration& config);

	//implementation of LocalDevice
	bool writeChannel(short channel, const STI::Utils::MixedValue& value);
	bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

	double getTestValue();

private:

	double testValue;

};


#endif

