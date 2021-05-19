
#include "ServerDevice.h"


#include "DeviceMessageReceiver.h"
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

ServerDevice::ServerDevice(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: STI::Device::LocalDevice(name, address, module, targetServer)
{
	STI::Engine::EngineID id(0);
	addEventEngine(id);

	addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");



///////////////////////////////////////
    std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

	auto l1 = std::make_shared<TempListener>("L1");
    auto l2 = std::make_shared<TempListener>("L2");
	
	STI::Device::DeviceMessageListenerID schedulerMessageLID;
    STI::Device::DeviceMessageListenerID schedulerMessageLID2;


    //EventEngineScheduler message listener
    auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(l1);
    
   	schedulerMessageLID.name = "::EventEngineScheduler::ParseResult";	//getID().getID() + 
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    auto listener2 = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(l2);
    
   	schedulerMessageLID2.name = "::EventEngineScheduler::ResultTicket";	//getID().getID() + 
	schedulerMessageLID2.type = STI::Device::DeviceMessageType::EngineScheduler;

	// std::cout << "Adding Listeners? ";
    // if (receiver != 0) {
	// 	std::cout << " add" << std::endl;
    //     receiver->addListener(getID(), schedulerMessageLID, listener);	//listen to events from server
    //     receiver->addListener(getID(), schedulerMessageLID2, listener2);	//listen to events from server
    // }

}

ServerDevice::~ServerDevice()
{
}

