#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>


class TestDevice : public STI::Device::LocalDevice
{
public:

	TestDevice(const STI::Utils::Configuration& config);

	//example attribute setter/refresher class functions
	bool setHeight(const std::string& value);
	std::string refreshHeight();

private:
	
	bool hardwareTrigger;
	int downsample;
	double height;
	int regionWidth;
	int regionHeight;

};


#endif

