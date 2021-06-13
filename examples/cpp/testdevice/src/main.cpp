
#include "TestDevice.h"
#include "NetworkDeviceHub.h"
//#include "STI_Network.h" "stinet.h"

#include <string>
#include <memory>

#include <iostream>

int main(int argc, char **argv)
{

//	std::cout << STI::Network::NetworkDeviceHub::printNetwork("192.168.1.4:2809", "STI") << std::endl;

	auto device = std::make_shared<TestDevice>("dev3", "localhost", 0, "localhost/0/STI Server");

    std::string nameServiceAddr = "192.168.1.4:2809";   //Address of OmniORB NameService (to connect to other Hubs)
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);

	hub->run(true);     //blocks until ctrl-c or Device terminates

	return 0;
}
