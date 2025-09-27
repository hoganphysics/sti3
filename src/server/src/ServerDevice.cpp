
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
	STI::Engine::EngineID id(0);
	addEventEngine(id);

	addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

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

