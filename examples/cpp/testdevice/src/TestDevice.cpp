

#include "TestDevice.h"

#include "EngineState.h"


#include <iostream>
#include <string>


class TempListener : public STI::Device::DeviceMessageListener<STI::Device::CollectionUpdateMessage>
{
public:

	TempListener(const std::string& name) : name(name) {}

	void handleMessage(const std::shared_ptr<STI::Device::CollectionUpdateMessage>& mess)
	{
		std::cout << "handle " << name << std::endl;
	}

	std::string name;
};

TestDevice::TestDevice(const STI::Device::Configuration& config)
: STI::Device::LocalDevice(config)
{
    init();
}

TestDevice::TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
: STI::Device::LocalDevice(name, address, module, targetServer)
{
    init();
}

void TestDevice::init()
{
    addChannel(1, 
               STI::Device::ChannelType::Output, 
               STI::Utils::MixedValueType::Double, 
               STI::Utils::MixedValueType::Double, 
               "test channel");

    STI::Engine::EngineID id(0);
    addEventEngine(id);
    
    STI::Device::DeviceID serverID;
    STI::Device::DeviceID::stringToDeviceID(getID().getTargetServerID(), serverID);

    addPartner(serverID);

    std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    auto l1 = std::make_shared<TempListener>("L1");
    
    collectionMessageLID.name = "::CollectionUpdateMessage::Test";	//getID().getID() + 
	collectionMessageLID.type = STI::Device::DeviceMessageType::CollectionUpdate;

    // receiver->addListener(serverID, collectionMessageLID, std::static_pointer_cast<STI::Device::DeviceMessageListener<STI::Device::CollectionUpdateMessage>>(l1));	//listen to events from server

    // receiver->addListener<STI::Device::CollectionUpdateMessage>(serverID, collectionMessageLID, 
    //     [](const std::shared_ptr<STI::Device::CollectionUpdateMessage>& message) { } );
    
    receiver->addListener<STI::Device::CollectionUpdateMessage>(serverID, collectionMessageLID, 
        [](auto message) { 
            std::cout << "handle functional " << STI::Device::DeviceMessage::typeToString(message->getMessageClassType()) << std::endl;
        } );

    
    jobMessageLID.name = "::JobUpdateMessage::Test";	//getID().getID() + 
	jobMessageLID.type = STI::Device::DeviceMessageType::EngineJobUpdate;

    receiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(serverID, jobMessageLID, 
        [](auto message) { 
            std::cout << "Job message: " << STI::Device::EngineJobUpdateDeviceMessage::jobTargetToString(message->getTargetList())// << STI::Device::DeviceMessage::typeToString(message->getMessageClassType())
            << " : " <<  message->getDeviceTrace().print() << " : " << message->getEngineJob()->getJobID().pid.parseTimestamp.print() << std::endl;
        } );

    stateMessageLID.name = "::EngineStateMessage::Test";	//getID().getID() + 
	stateMessageLID.type = STI::Device::DeviceMessageType::EngineStatus;

    receiver->addListener<STI::Device::EngineStateMessage>(serverID, stateMessageLID, 
        [](auto message) { 
            std::cout << "State message: " << STI::Engine::print((message->engineStates.begin()->second)) << std::endl;
        } );
}

TestDevice::~TestDevice()
{
}

void TestDevice::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) 
{
    for(auto& evt : events) {
        
        auto synchEvt = std::make_unique<TestEvent>(evt.second.at(0), this);
        synchedEvents.push_back(std::move(synchEvt));
    }
}

// Custom SynchronousEvent

TestDevice::TestEvent::TestEvent(const STI::Engine::RawEvent& evt, TestDevice* dev) 
: STI::Engine::SynchronousEventAdapter(evt.time()), evt(evt), localDevice(dev)
{
}
		
void TestDevice::TestEvent::playEvent()
{
    std::cout << "Play: " << localDevice->getID().getName() << " " << evt.print() << std::endl;
}
