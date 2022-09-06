#include <sti/sti.h>
#include <sti/LocalDevice.h>
#include <memory>

using STI::Device::LocalDevice;
using STI::Utils::Configuration;
using STI::Network::NetworkDeviceHub;


class SimpleDevice : public LocalDevice
{
public:

	SimpleDevice(const Configuration& config) : LocalDevice(config) {}
};


int main(int argc, char **argv)
{
	Configuration config({
		{"Device Name", "Simple Device"}, 
		{"IP Address", "localhost"}, 
		{"Module", "0"},
		{"Target Server", "localhost/0/STI Server"}});

	auto device = std::make_shared<SimpleDevice>(config);

	std::string nameServiceAddr = "192.168.1.4:2809";   //OmniORB NameService
	auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);
	hub->run();     	//blocks until ctrl-c or Device terminates
	hub->shutdown();

	return 0;
}
