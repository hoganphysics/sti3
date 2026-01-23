
#include <sti/sti.h>
// #include <sti/NetworkDeviceHub.h>
#include "ServerDevice.h"
#include "LegacyShotRepository.h"

#include "CLI11/CLI11.hpp"

#include <sti/device/LogFileFilter.h>

#include <sti/utils/Task.h>

#include <memory>

#include <iostream>

#include <sti/network/Node.h>

// #include "../../network/src/TTestNetwork_i.h"
// #include "../../network/src/ORBManager.h"
#include "TestNetworkWrapper.h"
#include "generated/deviceNet.h"


int main(int argc, char **argv)
{
    //command line parsing
    CLI::App app{"STI Server"};
    argv = app.ensure_utf8(argv);

    std::string configFilename = "default.ini";
    auto configFileOpt = app.add_option("-f,--file", configFilename, "STI Server configuration file")->check(CLI::ExistingFile);

    std::string address = "localhost:2809";
    auto nameserviceOpt = app.add_option("-n,--NameService", address, "NameService address string");

    CLI11_PARSE(app, argc, argv);

    STI::Utils::ConfigFile configFile(configFilename);

    std::shared_ptr<STI::Network::NetworkDeviceHub> hub;

    if (nameserviceOpt->count() > 0) {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>(address);
    }
    else if (configFileOpt->count() > 0) {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>(configFile);
    }
    else {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>("localhost:2809");
    }

	// hub = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.109:2809");
//    hub->getPersistenceOptions().bindToRootContext = false;
//    hub->getPersistenceOptions().bindToTargetContexts = false;


    // std::string testName = "STI Server";

    // auto server = std::make_shared<STI::Device::ServerDevice>(testName, "localhost", 0, "root");
    auto server = std::make_shared<STI::Device::ServerDevice>(configFile);

    

    auto legacyShotRepository = std::make_shared<STI::Engine::LegacyShotRepository>(
        configFile.get<std::string>("Shot Repository", "Path", ".sti/server1")
    );
    server->setShotRepository(legacyShotRepository);

    hub->addDevice(server);

    hub->run(true);	//non-blocking

    std::string enableActivateAt = configFile.get<std::string>("EnableActivate", "false");
    std::string enableDeactivateAt = configFile.get<std::string>("EnableDeactivate", "false");
    bool enableActivate = (enableActivateAt == "1" || enableActivateAt == "true");
    bool enableDeactivate = (enableDeactivateAt == "1" || enableDeactivateAt == "true");

    std::cout << "TestNetwork servant activate: " << (enableActivate ? "true" : "false") << ", deactivate: " << (enableDeactivate ? "true" : "false") << std::endl;
    ::STI::TNetwork::TTestNetwork_var testRef;

    {
        using STI::TNetwork::TestNetworkWrapper;
        TestNetworkWrapper wrapper(enableActivate, enableDeactivate);

        wrapper.getTestNetworkReference(testRef);
        // testRef->ping();

        // auto testRef = wrapper.test->_this();
    }


    // STI::Network::ORBManager::getInstance()->block();	//blocking



    //test logs
    if (false) {
        int x;
        std::cin >> x;

        std::shared_ptr<STI::Device::LogManager> lm;
        std::shared_ptr<STI::Device::DeviceCollection> collection;
        std::shared_ptr<STI::Device::Device> dev;

        server->getCollection(collection);
        auto testID = STI::Device::DeviceID("TestDevice", "localhost2", 0);
        collection->get(testID, dev);
        dev->getLogManager(lm);
        STI::Device::LogFileFilter filter;
        filter.startDate = "2023/09/02";
        filter.endDate = "2023/09/02";
        filter.logName = "ch1";
        filter.startIndex= 0;
        filter.endIndex = 2;
        auto count = lm->getLogCount(testID, filter);


        std::vector<STI::Device::LogID> ids;
        lm->getLogIDs(filter, ids);

        STI::Device::LogID logID;
        STI::Device::LogFile logFile;
        logID.date = "2023/09/02";
        logID.deviceID = testID;
        logID.logName = "ch1";
        logID.index = 0;

        auto success = lm->getLog(logID, logFile);

        auto logFilename = logFile.fileHolder->getFilename();
    }

    if(false) {
        int x;
        std::cin >> x;

        std::shared_ptr<STI::Device::TaskManager> tm;
        std::shared_ptr<STI::Device::DeviceCollection> collection;
        std::shared_ptr<STI::Device::Device> dev;
        std::shared_ptr<STI::Utils::Task> task;

        server->getCollection(collection);
        auto testID = STI::Device::DeviceID("TestDevice", "localhost2", 0);
        collection->get(testID, dev);
        dev->getTaskManager(tm);
        auto result = tm->getTask("Log::Read channel #2(test channel)", task);

        tm->setStatus(task->getID(), STI::Utils::TaskStatus::Inactive);
        
    }

    // hub->shutdown();

    return 0;
}

