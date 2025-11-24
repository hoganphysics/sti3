#include <sti/sti.h>
#include <sti/LocalDevice.h>

#include <memory>
#include <iostream>
#include <sstream>


using STI::Device::LocalDevice;
using STI::Utils::Configuration;
using STI::Utils::ConfigFile;
using STI::Network::NetworkDeviceHub;
using STI::Engine::SynchronousEventAdapter;


std::string printList(const std::vector<std::string>& items)
{
	std::stringstream s;

	bool first = true;
	s << "{";
	for (auto& item : items) {
		if (!first) {
			s << ", ";
		}
		s << item;
		first = false;
	}
	s << "}";

	return s.str();
}


class TestDevice : public LocalDevice
{
public:

	TestDevice(const Configuration& config) : LocalDevice(config)
	{
		std::string name = config.get<std::string>("Device Name", "");
		std::cout << "Configuring " << name << std::endl;

		int x;
		if (config.getParameter("SetupData", "x", x)) {
			std::cout << "x = " << x << std::endl;
		}

		double y = config.get("SetupData", "y", 2.5);	//default value 2.5 if "y" is not found
		std::cout << "y = " << y  << std::endl;

		auto states = config.getList("SetupData", "states");
		std::cout << "states: " << printList(states) << std::endl;

		auto listExample = config.getList("SetupData", "example");
		std::cout << "example: " << printList(listExample) << std::endl;
	}
};


int main(int argc, char** argv)
{
	std::string configFileName = "testDevice.ini";

	if (argc > 1) {
		configFileName = argv[1];
	}

	ConfigFile configFile(configFileName);

	std::string addr = configFile.get<std::string>("TestDevice1", "IP Address", "none");
	std::cout << "Test: " << addr << std::endl;

	auto device1 = std::make_shared<TestDevice>(configFile.extract("TestDevice1"));
	auto device2 = std::make_shared<TestDevice>(configFile.extract("TestDevice2"));

	auto hub = std::make_shared<NetworkDeviceHub>(configFile);	//automatically uses [NetworkHub] parameters to configure hub


	hub->addDevice(device1);
	hub->addDevice(device2);
	hub->run();     	//blocks until ctrl-c or Device terminates
	hub->shutdown();

	return 0;
}
