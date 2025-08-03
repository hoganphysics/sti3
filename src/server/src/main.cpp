
#include <sti/NetworkDeviceHub.h>
#include "ServerDevice.h"
#include "LegacyShotRepository.h"

#include <sti/device/LogFileFilter.h>

#include <sti/utils/Task.h>

#include <memory>

#include <iostream>

#include <sti/network/Node.h>

int main(int argc, char **argv)
{
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.109:2809");
//    hub->getPersistenceOptions().bindToRootContext = false;
//    hub->getPersistenceOptions().bindToTargetContexts = false;


    std::string testName = "STI Server";

    STI::Device::DeviceID id(testName, "localhost", 0, "root");

    auto server = std::make_shared<STI::Device::ServerDevice>(testName, "localhost", 0, "root");


    auto legacyShotRepository = std::make_shared<STI::Engine::LegacyShotRepository>(".sti/server1");
    server->setShotRepository(legacyShotRepository);

    hub->addDevice(server);

    hub->run(true);

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

    hub->shutdown();

    return 0;
}

