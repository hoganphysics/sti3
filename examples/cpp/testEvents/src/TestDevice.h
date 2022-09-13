
#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <sti/LocalDevice.h>

#include <string>
#include <mutex>
#include <iostream>


class TestDevice : public STI::Device::LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) 
        : STI::Device::LocalDevice(name, address, module, targetServer)
	{
		// std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
		// getEventDispatcher(dispatcher);
		// std::shared_ptr<STI::Device::DeviceCollection> collection;
		// getCollection(collection);
		
		// std::shared_ptr<STI::Engine::LocalEventEngineScheduler> scheduler;
		// getEngineScheduler(scheduler);
		
		// STI::Device::Channel ch(1, STI::Device::TChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");
		// //channels[1] = ch;

		// localChannels[1] = ch;

        addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

		addAttribute("attrib_" + name, "34");

		STI::Engine::EngineID id(0);
//		auto engine = std::make_shared<STI::Engine::LocalEventEngine>(getID(), channels, this, dispatcher, collection);
//		scheduler->addEngine(id, engine);

		addEventEngine(id);

	}
	~TestDevice()
	{
		std::cout << "Destroying " << getID().getName() << std::endl;
	}

	// void addEventTarget(const STI::Device::DeviceID& id)
	// {
	// 	eventTargets.insert(id);
	// }

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) 
	{
		std::cout << "Parsing: " << getID().getName() << std::endl;
		std::cout << "Event count: " << events.size() << std::endl;
		
		if (events.size() > 0) {
			std::cout << events.begin()->second.at(0).print() << std::endl;
		}

		auto evt = std::make_unique<TestEvent>(events.begin()->second.at(0), this);
		synchedEvents.push_back(std::move(evt));
	}

	class TestEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:

		TestEvent(const STI::Engine::RawEvent& evt, TestDevice* dev) 
		: STI::Engine::SynchronousEventAdapter(evt.time()), evt(evt), localDevice(dev) {}
		
		void playEvent()
		{
			std::unique_lock<std::mutex> writeLock(TestEvent::coutMutex);
			std::cout << "Play: " << localDevice->getID().getName() << " " << evt.print() << std::endl;
		}

		STI::Engine::RawEvent evt;
		STI::Device::LocalDevice* localDevice;

		static std::mutex coutMutex;
	};

	//STI::Device::ChannelMap channels;

};



#endif

