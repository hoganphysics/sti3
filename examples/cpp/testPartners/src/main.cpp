
#include "LocalDevice.h"
#include "NetworkDeviceHub.h"
//#include "STI_Network.h" "stinet.h"

#include "LocalDeviceHub.h"
//#include "LocalEventEngineScheduler.h"
#include "EventEngineScheduler.h"
#include "LocalShot.h"
#include "SynchronousEvent.h"

#include <string>
#include <memory>
#include <iostream>

class DigitalDevice : public STI::Device::LocalDevice
{
public:
	DigitalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
	: STI::Device::LocalDevice(name, address, module, targetServer),
		triggerID("Trigger", "localhost", 0, "localhost/0/STI Server")
	{
		STI::Engine::EngineID id0(0);
		addEventEngine(id0);

		addPartner(triggerID);

		addChannel(0, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Boolean, "");
	}
	~DigitalDevice() {}

	class DigitalEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:
		DigitalEvent(double time) : STI::Engine::SynchronousEventAdapter(time) {}
		void playEvent()
		{
			std::cout << "Play Digital" << std::endl;
		}
	};

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
	{
		std::cout << "Parse Digital" << std::endl;

		auto& refEvt = events.begin()->second.at(0);
		STI::Engine::RawEvent triggerEvent(triggerID, 0, 0, "Play", "", 0, STI::Engine::RawEventType::Play);
		addEvent(triggerEvent, refEvt);

		auto evt = std::make_unique<DigitalEvent>(0);
		synchedEvents.push_back(std::move(evt));
	}

	STI::Device::DeviceID triggerID;
};


class TriggerDevice : public STI::Device::LocalDevice
{
public:
	TriggerDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
		: STI::Device::LocalDevice(name, address, module, targetServer)
	{
		STI::Engine::EngineID id0(0);
		addEventEngine(id0);

		addChannel(0, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::String, "trigger");
	}
	~TriggerDevice() {}
	
	class TriggerEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:
		TriggerEvent(double time) : STI::Engine::SynchronousEventAdapter(time) {}
		void playEvent()
		{
			std::cout << "Play Trigger" << std::endl;
		}
	};

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
	{
		std::cout << "Parse Trigger" << std::endl;

		auto evt = std::make_unique<TriggerEvent>(0);
		synchedEvents.push_back(std::move(evt));
	}

};


int main(int argc, char **argv)
{

	auto digital = std::make_shared<DigitalDevice>("Digital", "localhost", 0, "localhost/0/STI Server");
	auto trigger = std::make_shared<TriggerDevice>("Trigger", "localhost", 0, "localhost/0/STI Server");

	auto hub = std::make_shared<STI::Network::LocalDeviceHub>("TestHub", "localhost", 0);
	hub->addDevice(digital);
	hub->addDevice(trigger);
	
	int x;
	std::cin >> x;

	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	digital->getEngineScheduler(scheduler);

	STI::Engine::ParseID parseID;
	auto shot = std::make_shared<STI::Engine::LocalShot>();
	auto events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	STI::Engine::RawEvent evt0(digital->getID(), 0, 0, true, "", 0, STI::Engine::RawEventType::Play);
	events->push_back(evt0);

	shot->setEvents(events);
	scheduler->parse(parseID, shot);

	std::cin >> x;

	STI::Engine::ShotID sid;
	sid.parseID = parseID;
	scheduler->play(sid);

	std::cin >> x;

	return 0;
}
