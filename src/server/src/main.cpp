
#include <sti/NetworkDeviceHub.h>
#include "ServerDevice.h"
#include "LegacyShotRepository.h"

#include <memory>

#include <iostream>

int main(int argc, char **argv)
{
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.14:2809");
//    hub->getPersistenceOptions().bindToRootContext = false;
//    hub->getPersistenceOptions().bindToTargetContexts = false;

    std::string testName = "STI Server";

    STI::Device::DeviceID id(testName, "localhost", 0, "root");

    auto server = std::make_shared<STI::Device::ServerDevice>(testName, "localhost", 0, "root");


    auto legacyShotRepository = std::make_shared<STI::Engine::LegacyShotRepository>(".sti/server1");
    server->setShotRepository(legacyShotRepository);

    hub->addDevice(server);

    hub->run(true);

    //auto hub2 = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.14:2809");

    //STI::Network::HubID hubID;
    //
    //hub2->run(false);


    //hub2->findHub(id, hubID);


    

    // int x;
    // std::cin >> x;

    // server = 0;
    // hub->shutdown();

    // std::cin >> x;

    return 0;
}

