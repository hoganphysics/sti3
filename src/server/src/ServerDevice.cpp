
#include "ServerDevice.h"


#include <sti/device/DeviceMessageReceiver.h>
#include <iostream>

using STI::Device::ServerDevice;

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

// ServerDevice::ServerDevice(const std::string& name, const std::string& address, unsigned short module,
//     const std::string& targetServer)
// : STI::Device::LocalDevice(name, address, module, targetServer)
ServerDevice::ServerDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// STI::Engine::EngineID id(0);
	// addEventEngine(id);

	addChannel(0, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

	partnerID = STI::Device::DeviceID("TestDevice2", "localhost", 2, "sr-magis/2/Frame2");
	addEventTarget(partnerID);

	// std::shared_ptr<AttributeManager> attributeManager;
	// getAttributeManager(attributeManager);
	addAttribute("test", "45");

	std::shared_ptr<AttributeManager> attributeManager;
	getAttributeManager(attributeManager);
	std::string testAt = attributeManager->getValue("test");

///////////////////////////////////////
    std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

	// auto l1 = std::make_shared<TempListener>("L1");
    // auto l2 = std::make_shared<TempListener>("L2");
	
	// STI::Device::DeviceMessageListenerID schedulerMessageLID;
    // STI::Device::DeviceMessageListenerID schedulerMessageLID2;


    // //EventEngineScheduler message listener
    // auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(l1);
    
   	// schedulerMessageLID.name = "::EventEngineScheduler::ParseResult";	//getID().getID() + 
	// schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    // auto listener2 = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(l2);
    
   	// schedulerMessageLID2.name = "::EventEngineScheduler::ResultTicket";	//getID().getID() + 
	// schedulerMessageLID2.type = STI::Device::DeviceMessageType::EngineScheduler;

	// std::cout << "Adding Listeners? ";
    // if (receiver != 0) {
	// 	std::cout << " add" << std::endl;
    //     receiver->addListener(getID(), schedulerMessageLID, listener);	//listen to events from server
    //     receiver->addListener(getID(), schedulerMessageLID2, listener2);	//listen to events from server
    // }


	// STI::Device::DeviceID tmpDeviceID("dev3", "localhost", 0, "localhost/0/STI Server");
	// STI::Device::DeviceMessageListenerID channelMessageLID;
	// channelMessageLID.name = "::ChannelUpdateMessage::dev3";
	// channelMessageLID.type = STI::Device::DeviceMessageType::ChannelUpdate;
    // receiver->addListener<STI::Device::ChannelUpdateMessage>(tmpDeviceID, channelMessageLID, 
    //     [](auto message) { 
    //         std::cout << "New value: " << message->channelValues[1].print() << std::endl;
    //     } );

	// receiver->addListener<STI::Device::CollectionUpdateMessage>(getID(), "LocalCollectionLister",
	// 	[this](auto& message) {
	// 		std::cout << "Collection: " << message->sourceID().getID() << std::endl;
	// 		std::shared_ptr<STI::Device::DeviceCollection> collection;
	// 		this->getCollection(collection);
	// 		DeviceID id;
	// 		DeviceID::stringToDeviceID("localhost/0/TestDevice", id);
	// 		std::shared_ptr<STI::Device::Device> device;
			
	// 		std::cout << "collection->get " << (collection->get(id, device) ? "1" : "0") << std::endl;
	// 		std::cout << "device ? " << (device != 0 ? "1" : "0") << std::endl;

	// 		// if (device != 0) {
	// 		// 	std::cout << "write:" << std::endl;
	// 		// 	std::shared_ptr<ChannelManager> manager;
	// 		// 	device->getChannelManager(manager);
	// 		// 	manager->writeChannel(1, 2.2);
	// 		// 	std::cout << "write complete" << std::endl;
	// 		// }
	// 	});

		// DeviceID testID;
		// DeviceID::stringToDeviceID("localhost/0/TestDevice", testID);

		// receiver->addListener<STI::Device::AttributeUpdateMessage>(testID,"server attribute update listener",
		// 	[this](auto& message) {
		// 		std::cout << "AttributeUpdateMessage: " << message->toString() << std::endl;
		// 	}
		// );

}

ServerDevice::~ServerDevice()
{
}

// bool ServerDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
// {
// 	std::cout << "ServerDevice::writeChannel " << channel << " value: " << value.print() << std::endl;

// 	return true;
// }



void ServerDevice::parseEvents(const STI::Engine::RawEventMap& eventsIn, STI::Engine::SynchronousEventVector& synchedEvents)
{
	bool inputEvent;

	STI::Engine::RawEventTargetChannel partnerCh(0);

	for (auto& tuple : eventsIn) {
		// Here 'tuple' is of type { time, vector<RawEvent> }, where the vector is the set of events at this time.
		inputEvent = false;

		//check for input event
		for (auto& rawEvent : tuple.second) {
			if (rawEvent.isMeasurementEvent()) {
				inputEvent = true;
				break;
			}
		}

		// Example of error checking. When an error is encountered during parsing, an exception should be thrown.
		// This exception is handled by STI and converted to a parsing error message. 
		if (inputEvent && tuple.second.size() > 1) {
			// Error: In this example, when there is an input event, it must be the only event at that time.
			// The EventConflictException constuctor accepts reference to the two offending events, plus a message: 
			throw STI::Engine::EventConflictException(tuple.second.at(0), tuple.second.at(1), 
				"Error: An input event cannot be at the same time as an output event.");
		}

		if (!inputEvent) {
			//Output event
			auto testDeviceOutputEvent = std::make_shared<ServerDevice::TestDeviceOutputEvent>(tuple.first);	//time
			
			for (auto& rawEvent : tuple.second) {
				
				if (rawEvent.value().getNumber() > 10) {
					throw STI::Engine::EventParsingException(rawEvent, 
					"The value " + rawEvent.value().print() + " exceeds the maximium allowed value for this channel. Max value is 10.");
				}
				testDeviceOutputEvent->addValue(rawEvent.channel(), rawEvent.value());	//attach values to event for use later

				if (rawEvent.channel() == 0) {
                	// Example of sending a partner event when this device has an output event on channel 0 
                    partner(partnerID).addEvent(tuple.first, partnerCh, 3.2*rawEvent.value().getNumber(), rawEvent);
				}

			}
			synchedEvents.push_back(testDeviceOutputEvent);
		}
	}
}



// //////////////// TestDeviceOutputEvent //////////////////

// This subclass is used to specify the custom behavior controlling the hardware for each device output channel.

ServerDevice::TestDeviceOutputEvent::TestDeviceOutputEvent(double time)
: STI::Engine::SynchronousEvent(time)
{
}

void ServerDevice::TestDeviceOutputEvent::addValue(short channel, const STI::Utils::MixedValue& value)
{
	values[channel] = value;
}

void ServerDevice::TestDeviceOutputEvent::loadEvent()
{
	// This function is called at the beginning of each shot, before any events are played.
	// Use this function to setup the hardware to prepare for hard timing playback.
	// For example, if this device requires values to be preloaded into some buffer on the hardware
	// (e.g., an FPGA, or an arbitrary waveform generator), this can be done here.

	// Add hardware loading code here...
	for (auto& v : values) {
		std::cout << "Loading channel #" << v.first << " with value " << v.second.getNumber() << "." << std::endl;
	}
}

void ServerDevice::TestDeviceOutputEvent::playEvent()
{
	// This function will be called at time specified in the timing file.
	// Use this function to control the hardware to implement the change on the requested channel.

	// Add hardware play code here...
	for (auto& v : values) {
		std::cout << "Playing channel #" << v.first << " with value " << v.second.getNumber() << "." << std::endl;
	}
}

