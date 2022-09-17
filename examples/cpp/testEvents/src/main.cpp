#include <sti/sti.h>

#include "TestDevice.h"

#include <sti/engine/RawEventGroup.h>

#include <memory>
#include <iostream>

using STI::Device::LocalDevice;
using STI::Utils::Configuration;
using STI::Network::NetworkDeviceHub;

std::mutex TestDevice::TestEvent::coutMutex{};

int main(int argc, char **argv)
{
	auto dev1 = std::make_shared<TestDevice>("dev1", "localhost", 0, "srv1");
	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/dev1");
	auto dev3 = std::make_shared<TestDevice>("dev3", "localhost", 0, "localhost/0/dev2");
	auto dev4 = std::make_shared<TestDevice>("dev4", "localhost", 0, "localhost/0/dev1");

	STI::Network::HubID hubID("localhub", "localhost", 0);

	auto hub1 = std::make_shared<STI::Network::LocalDeviceHub>(hubID);

	hub1->addDevice(dev1);
	hub1->addDevice(dev2);
	hub1->addDevice(dev3);
	hub1->addDevice(dev4);

	int x;
	std::cin >> x;


	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	dev1->getEngineScheduler(scheduler);

	STI::Engine::ShotConfig config;
	config.shotType = STI::Engine::ShotType::Single;
	auto group = std::make_shared<STI::Engine::RawEventGroup>();

	STI::Utils::MixedValue value;
	value.setValue(27.0);
	group->addEvent(STI::Engine::RawEventTarget(dev1->getID(), 1), 2.01, value, STI::Engine::RawEventType::Play);
	group->addEvent(STI::Engine::RawEventTarget(dev2->getID(), 1), 3.01, value, STI::Engine::RawEventType::Play);
	group->addEvent(STI::Engine::RawEventTarget(dev3->getID(), 1), 4.01, value, STI::Engine::RawEventType::Play);
	group->addEvent(STI::Engine::RawEventTarget(dev4->getID(), 1), 5.01, value, STI::Engine::RawEventType::Play);

	auto shot = scheduler->createShot(config, group);

	auto pid = scheduler->parse(shot);

	std::cin >> x;

	STI::Engine::EngineJobSourceID sourceID;
	auto sid = scheduler->play(pid, sourceID);

	std::cin >> x;

	return 0;
}
