
#include "LocalDevice.h"
#include "NetworkDeviceHub.h"
//#include "STI_Network.h" "stinet.h"

#include "LocalDeviceHub.h"
//#include "LocalEventEngineScheduler.h"
#include "EventEngineScheduler.h"
#include "LocalShot.h"
#include "SynchronousEvent.h"
#include "Measurement.h"

#include <string>
#include <memory>
#include <iostream>

class AnalogInDevice : public STI::Device::LocalDevice
{
public:
	AnalogInDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
	: STI::Device::LocalDevice(name, address, module, targetServer),
		triggerID("Trigger", "localhost", 0, "localhost/0/STI Server")
	{
		STI::Engine::EngineID id0(0);
		addEventEngine(id0);

		addPartner(triggerID);

		addChannel(0, STI::Device::ChannelType::Input, STI::Utils::MixedValueType::Double, STI::Utils::MixedValueType::Double, "");

		addAttribute("settings::testAttrib", "the value of the attribute");
	}
	~AnalogInDevice() {}

	class AnalogInEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:
		AnalogInEvent(double time) : STI::Engine::SynchronousEventAdapter(time) {}
		void playEvent()
		{
			std::cout << "Play AnalogIn" << std::endl;
		}
		void collectMeasurementData()
		{
			getMeasurements().at(0)->setMeasurementResult(22.3);
		}
	};

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
	{
		std::cout << "Parse AnalogIn" << std::endl;

		auto& refEvt = events.begin()->second.at(0);
		STI::Engine::RawEvent triggerEvent(triggerID, 0, 0, "Play", "", 0, STI::Engine::RawEventType::Play);
		addEvent(triggerEvent, refEvt);

		auto evt = std::make_unique<AnalogInEvent>(0);
		evt->addMeasurement(refEvt);
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
		addChannel(1, STI::Device::ChannelType::Input, STI::Utils::MixedValueType::String, STI::Utils::MixedValueType::Empty, "");
	}
	~TriggerDevice() {}
	
	class TriggerEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:
		TriggerEvent(double time, int channel) : STI::Engine::SynchronousEventAdapter(time), channel(channel) {}
		void playEvent()
		{
			std::cout << "Play Trigger" << std::endl;
		}
		void collectMeasurementData()
		{
			if (channel == 1)
				getMeasurements().at(0)->setMeasurementResult("test");
		}
		int channel;
	};

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
	{
		std::cout << "Parse Trigger" << std::endl;

		auto evt = std::make_unique<TriggerEvent>(0,0);
		synchedEvents.push_back(std::move(evt));

		auto ev = events.begin();
		ev++;
		
		auto evt2 = std::make_unique<TriggerEvent>(ev->first, 1);
		evt2->addMeasurement(ev->second.at(0));
		synchedEvents.push_back(std::move(evt2));
	}

};


int main(int argc, char **argv)
{

	auto analogin = std::make_shared<AnalogInDevice>("AnalogIn", "localhost", 0, "localhost/0/STI Server");
	auto trigger = std::make_shared<TriggerDevice>("Trigger", "localhost", 0, "localhost/0/STI Server");

	auto hub = std::make_shared<STI::Network::LocalDeviceHub>("TestHub", "localhost", 0);
	hub->addDevice(analogin);
	hub->addDevice(trigger);
	
	int x;
	std::cin >> x;

	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	analogin->getEngineScheduler(scheduler);

	STI::Engine::ParseID parseID;
	auto shot = std::make_shared<STI::Engine::LocalShot>();
	auto events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	STI::Engine::RawEvent evt0(analogin->getID(), 0, 0, 12.0, "", 0, STI::Engine::RawEventType::Measurement);
	events->push_back(evt0);


	//STI::Utils::MixedValueType::Empty
	STI::Engine::RawEvent evt1(trigger->getID(), 50, 1, STI::Utils::MixedValueType::Empty, "", 0, STI::Engine::RawEventType::Measurement);
	events->push_back(evt1);

	shot->setEvents(events);
	scheduler->parse(parseID, shot);

	std::cin >> x;

	std::vector<STI::Engine::EngineParsingMessage> messages;
	scheduler->getParsingMessages(parseID, messages);
	for (auto& m : messages) {
		std::cout << "Message: " << m.getMessage() << std::endl;
	}

	STI::Engine::ShotID sid;
	sid.parseID = parseID;
	scheduler->play(sid);

	std::cin >> x;

	return 0;
}
