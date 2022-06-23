
#include <sti/LocalDevice.h>
#include <sti/NetworkDeviceHub.h>
//#include "STI_Network.h" "stinet.h"

#include <sti/LocalDeviceHub.h>
//#include "LocalEventEngineScheduler.h"
#include <sti/engine/EventEngineScheduler.h>
#include "LocalShot.h"
#include <sti/engine/SynchronousEvent.h>
#include <sti/engine/Measurement.h>

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

		addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "");

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

	//void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}

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

	void parseEvents2(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
	{
		std::cout << "Parse Trigger" << std::endl;

		for (auto& ev : events) {
			auto evt = std::make_unique<TriggerEvent>(ev.first, ev.second.front().channel());

			if (ev.second.front().channel() == 1) {
				evt->addMeasurement(ev.second.front());
			}

			synchedEvents.push_back(std::move(evt));
		}

		// auto evt = std::make_unique<TriggerEvent>(0,0);
		// synchedEvents.push_back(std::move(evt));

		// auto ev = events.begin();
		// ev++;
		
		// auto evt2 = std::make_unique<TriggerEvent>(ev->first, 1);
		// evt2->addMeasurement(ev->second.at(0));
		// synchedEvents.push_back(std::move(evt2));
	}

};


int main(int argc, char **argv)
{
	auto analogin = std::make_shared<AnalogInDevice>("AnalogIn", "localhost", 0, "localhost/0/STI Server");
	auto trigger = std::make_shared<TriggerDevice>("Trigger", "localhost", 0, "localhost/0/STI Server");

	auto hub = std::make_shared<STI::Network::LocalDeviceHub>("TestHub", "localhost", 0);
	hub->addDevice(analogin);
	hub->addDevice(trigger);
	
	auto writeCheck = analogin->write(1, 3.2);

	double tmp = 44;

	int x;
	std::cin >> x;

	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	analogin->getEngineScheduler(scheduler);

	STI::Engine::ShotConfig shotConfig;
	shotConfig.shotType = STI::Engine::ShotType::Single;
	auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig);

	STI::Engine::RawEvent evt0(analogin->getID(), 0, 0, 12.0, "", 0, STI::Engine::RawEventType::Measurement);
	shot->addEvent(evt0);

	//STI::Utils::MixedValueType::Empty
	STI::Engine::RawEvent evt1(trigger->getID(), 50, 1, STI::Utils::MixedValueType::Empty, "", 0, STI::Engine::RawEventType::Measurement);
	shot->addEvent(evt1);

	auto parseID = scheduler->parse(shot);

	std::cin >> x;

	std::vector<STI::Engine::EngineParsingMessage> messages;
	scheduler->getParsingMessages(parseID, messages);
	for (auto& m : messages) {
		std::cout << "Message: " << m.getMessage() << std::endl;
	}

	auto sid = scheduler->play(parseID, parseID.shotConfig.jobSourceID);

	std::cin >> x;

	STI::Utils::MixedValue data;
	analogin->read(0, 2.2, data);

	std::cout << "Data = " << data.print() << std::endl;

	std::cin >> x;

	auto res = analogin->write(1, 3.2);

	return 0;
}
