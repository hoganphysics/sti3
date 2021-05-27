
#include "NetworkDeviceHub.h"
#include "ServerDevice.h"

#include <memory>


int main(int argc, char **argv)
{
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.4:2809");
//    hub->getPersistenceOptions().bindToRootContext = false;
//    hub->getPersistenceOptions().bindToTargetContexts = false;

    auto server = std::make_shared<STI::Device::ServerDevice>("STI Server", "localhost", 0, "root");

    hub->addDevice(server);

    hub->run(true);

    return 0;
}

