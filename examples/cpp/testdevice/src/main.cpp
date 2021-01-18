
#include "TestDevice.h"
#include "NetworkDeviceHub.h"
//#include "STI_Network.h" "stinet.h"

#include <string>
#include <memory>

int main(int argc, char **argv)
{
	auto device = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/dev1");

    std::string nameServiceAddr = "192.168.1.4:2809";   //Address of OmniORB NameService (to connect to other Hubs)
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);

	hub->run(true);     //blocks until ctrl-c or Device terminates

	return 0;
}
