
#include <sti/sti.h>
#include "TestDevice.h"


#include <string>
#include <memory>
#include <iostream>

#include <sti/utils/IntervalTask.h>
#include <sti/utils/AppointmentTask.h>
#include <sti/utils/TaskScheduler.h>



int main(int argc, char **argv)
{
	std::string configFilename = "config.ini"; //default

	if (argc > 1) {
		configFilename = argv[1];
	}

	STI::Utils::ConfigFile config(configFilename);
    std::string nameServiceAddr = "192.168.1.109:2809";   //Address of OmniORB NameService (to connect to other Hubs)
	
	auto device = std::make_shared<TestDevice>(config);

	// auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr, config);
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);

	STI::Utils::TaskScheduler scheduler;
	int x = 0;
	auto task1 = std::make_shared<STI::Utils::IntervalTask>("0", "00:00:02", //"00:00:02"
	[&x](){
		std::cout << "Task " << (++x) << std::endl; 
	});
	auto task2 = std::make_shared<STI::Utils::AppointmentTask>("1", "17:57:10", 
	[&x](){
		std::cout << "*** Task 2 ****" << std::endl; 
	});
	// scheduler.start();
	// scheduler.addTask(task1);
	// scheduler.addTask(task2);
	// scheduler.start();


	hub->run(true);     //blocks until ctrl-c or Device terminates

	if (false) {
		std::cin >> x;
		std::shared_ptr<STI::Device::LogManager> lm;
		device->getLogManager(lm);
		STI::Device::LogFileFilter filter;
		filter.startDate = "2023/09/03";
		filter.endDate = "2023/09/03";
		auto count = lm->getLogCount(device->getID(), filter);
	}

	hub->shutdown();
	// int x;
	// std::cin >> x;

	return 0;
}
