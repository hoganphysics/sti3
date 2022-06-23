

#include <sti/NetworkDeviceHub.h>
#include <sti/LocalDeviceHub.h>

#include <sti/LocalDevice.h>

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceCollection.h>
#include "LocalEventEngineScheduler.h"
#include "LocalEventEngine.h"
#include <sti/engine/EngineID.h>
#include <sti/device/Channel.h>
#include <sti/engine/ParseID.h>
#include "LocalShot.h"
#include <sti/engine/RawEvent.h>
#include <sti/device/Channel.h>

#include <sti/engine/ShotID.h>

#include <sti/engine/SynchronousEvent.h>

#include "Convert_EventEngine.h"

#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessage.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

using std::cout;
using std::endl;
using STI::Device::LocalDevice;


class TempListener : public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

	TempListener(const std::string& name) : name(name) {}

	void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
	{
		std::cout << "handle " << name << std::endl;
	}

	std::string name;
};


class TestDevice : public LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : LocalDevice(name, address, module, targetServer)
	{
		// std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
		// getEventDispatcher(dispatcher);
		// std::shared_ptr<STI::Device::DeviceCollection> collection;
		// getCollection(collection);
		
		// std::shared_ptr<STI::Engine::LocalEventEngineScheduler> scheduler;
		// getEngineScheduler(scheduler);
		
		addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

		
		//STI::Device::Channel ch(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");
		//channels[1] = ch;
		//localChannels[1] = ch;

		STI::Engine::EngineID id(0);
		addEventEngine(id);

//		auto engine = std::make_shared<STI::Engine::LocalEventEngine>(getID(), channels, this, dispatcher, collection);
//		scheduler->addEngine(id, engine);






///////////////////////////////////////
    std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

	auto l1 = std::make_shared<TempListener>("L1");
    auto l2 = std::make_shared<TempListener>("L2");
	
	STI::Device::DeviceMessageListenerID schedulerMessageLID;
    STI::Device::DeviceMessageListenerID schedulerMessageLID2;


    //EventEngineScheduler message listener
    auto listener = std::static_pointer_cast<STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>>(l1);
    
   	schedulerMessageLID.name = "::EventEngineScheduler::ParseResult";	//getID().getID() + 
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    auto listener2 = std::static_pointer_cast<STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>>(l2);
    
   	schedulerMessageLID2.name = "::EventEngineScheduler::ResultTicket";	//getID().getID() + 
	schedulerMessageLID2.type = STI::Device::DeviceMessageType::EngineScheduler;

	// std::cout << "Adding Listeners? ";
    // if (receiver != 0) {
	// 	std::cout << " add" << std::endl;
    //     receiver->addListener(getID(), schedulerMessageLID, listener);	//listen to events from server
    //     receiver->addListener(getID(), schedulerMessageLID2, listener2);	//listen to events from server
    // }



	}
	~TestDevice()
	{
		cout << "Destroying " << getID().getName() << endl;
	}
	void tmp()
	{
		STI::Device::DeviceID sid("dev1", "localhost", 0);
		addPartner(sid);
	}


	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) 
	{
		cout << "Parsing: " << getID().getName() << endl;
		cout << "Event count: " << events.size() << endl;
		
		if (events.size() > 0) {
			cout << events.begin()->second.at(0).print() << endl;
			auto evt = std::make_unique<TestEvent>(events.begin()->second.at(0), this);
			synchedEvents.push_back(std::move(evt));
		}

	}

	class TestEvent : public STI::Engine::SynchronousEventAdapter
	{
	public:

		TestEvent(const STI::Engine::RawEvent& evt, TestDevice* dev) 
		: STI::Engine::SynchronousEventAdapter(evt.time()), evt(evt), localDevice(dev) {}
		
		void playEvent()
		{
			std::unique_lock < std::mutex > writeLock(TestEvent::coutMutex);
			cout << "Play: " << localDevice->getID().getName() << " " << evt.print() << endl;
		}

		STI::Engine::RawEvent evt;
		STI::Device::LocalDevice* localDevice;

		static std::mutex coutMutex;
	};

	//STI::Device::ChannelMap channels;

};

std::mutex TestDevice::TestEvent::coutMutex{};


void testDevice();
void testServer();

int main(int argc, char **argv)
{
	testDevice();
	
	int select;
	std::cout << "(1) Server, (2) Device: ";
	std::cin >> select;
	std::cout << endl;

	switch (select)
	{
	case 1:
		testServer();
		break;
	case 2:
		testDevice();
		break;	
	default:
		break;
	}

	return 0;
}

void testDevice()
{
//	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/dev1");
	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/STI Server");

	auto hub1 = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.4:2809");

	dev2->getID();

//	dev2->tmp();

	hub1->addDevice(dev2);

	hub1->run(true);

	// int x;
	// std::cin >> x;

	//STI::Engine::ParseID pid;
	// pid.parseTimestamp.timestamp = 1.1;

	//std::vector<STI::Engine::EngineParsingMessage> messages;


	// //check remote access
	// STI::Device::DeviceID sid("dev1", "localhost", 0);
	// std::shared_ptr<STI::Device::DeviceCollection> dcollection;
	// std::shared_ptr<STI::Device::Device> server;
	// std::shared_ptr<STI::Engine::EventEngineScheduler> remoteScheduler;
	// dev2->getCollection(dcollection);
	// dcollection->get(sid, server);
	// server->getEngineScheduler(remoteScheduler);

	// messages.clear();
	//remoteScheduler->getParsingMessages(pid, messages);

}

void testServer()
{

	STI::Device::DeviceID id2("dev2", "localhost", 0, "localhost/0/STI Server");

	auto dev1 = std::make_shared<TestDevice>("STI Server", "localhost", 0, "root");
//	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/STI Server");
	auto dev3 = std::make_shared<TestDevice>("dev3", "localhost", 0, "localhost/0/dev2");
	auto dev4 = std::make_shared<TestDevice>("dev4", "localhost", 0, "localhost/0/STI Server");

	dev4->tmp();


	STI::Engine::ShotConfig shotConfig0;
	auto shot0 = std::make_shared<STI::Engine::LocalShot>(shotConfig0);
	STI::Utils::MixedValue value0;
	value0.setValue(28.0);
	auto evt0 = STI::Engine::RawEvent(dev1->getID(), 2.01, 1, value0, "desc", 0, STI::Engine::RawEventType::Play);

	STI::TNetwork::TRawEvent tRawEvent0;
	STI::Network::convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(evt0, tRawEvent0);

	std::shared_ptr<STI::Engine::RawEventVector> events0;
	shot0->getEvents(events0);
	events0->push_back(evt0);
	events0->push_back(evt0);

	STI::TNetwork::TRawEventSeq_var tRawEvents0 = new STI::TNetwork::TRawEventSeq();

	//STI::TNetwork::TRawEventSeq_var tRawEvtseq_var(new STI::TNetwork::TRawEventSeq);

	STI::Network::convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(*events0, 
						(_CORBA_Unbounded_Sequence<STI::TNetwork::TRawEvent>&) tRawEvents0);


//	auto hub1 = std::make_shared<STI::Network::LocalDeviceHub>("Hub1");
//	auto hub2 = std::make_shared<STI::Network::LocalDeviceHub>("Hub2");
//	STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>::connect(hub1, hub2);

	auto hub1 = std::make_shared<STI::Network::NetworkDeviceHub>("192.168.1.4:2809");
	//STI::Network::NetworkDeviceHub hub1("192.168.1.6:2809");
	//hub1.addNode(dev2->getID(), dev2);


	hub1->addDevice(dev1);
//	hub1->addNode(dev2->getID(), dev2);
	hub1->addDevice(dev3);
	hub1->addDevice(dev4);


	hub1->run(false);

	int x;
	std::cin >> x;


	//dev3->addEventTarget(dev1->getID());

	STI::Engine::ShotConfig shotConfig;
	auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig);

	STI::Utils::MixedValue value;
	value.setValue(27.0);
	auto evt1 = STI::Engine::RawEvent(dev1->getID(), 2.01, 1, value, "desc", 0, STI::Engine::RawEventType::Play);
	auto evt2 = STI::Engine::RawEvent(id2, 3.01, 1, value, "desc2", 1, STI::Engine::RawEventType::Play);
	auto evt3 = STI::Engine::RawEvent(dev3->getID(), 4.01, 1, value, "desc3", 2, STI::Engine::RawEventType::Play);
	auto evt4 = STI::Engine::RawEvent(dev4->getID(), 5.01, 1, value, "desc4", 3, STI::Engine::RawEventType::Play);

	std::shared_ptr<STI::Engine::RawEventVector> events;
	shot->getEvents(events);

	events->push_back(evt1);
	events->push_back(evt2);
	events->push_back(evt3);
	events->push_back(evt4);

	std::cout << "Length events: " << events->size() << std::endl;

	std::shared_ptr<STI::Engine::LocalEventEngineScheduler> scheduler;
	
	dev1->getEngineScheduler(scheduler);

	std::cin >> x;
	
	STI::Engine::ParseID pid = scheduler->parse(shot);

//	hub2->addNode(dev3->id, dev3);
//	hub2->addNode(dev4->id, dev4);


	std::cin >> x;

	std::vector<STI::Engine::EngineParsingMessage> messages;
	scheduler->getParsingMessages(pid, messages);


	//check remote access
	std::shared_ptr<STI::Device::DeviceCollection> dcollection;
	std::shared_ptr<STI::Device::Device> server;
	std::shared_ptr<STI::Engine::EventEngineScheduler> remoteScheduler;
	dev4->getCollection(dcollection);
	
	if (dcollection->get(dev1->getID(), server) && server != 0) {
		server->getEngineScheduler(remoteScheduler);

		messages.clear();
		remoteScheduler->getParsingMessages(pid, messages);
	}
	

	STI::Engine::ShotID shotID = scheduler->play(pid, pid.shotConfig.jobSourceID);

	std::cin >> x;

//	hub1->clear();

}

