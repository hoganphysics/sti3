
#include "TestDevice.h"

#include <iostream>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Device::DeviceMessageReceiver;


class TestDeviceChannelUpdateListener : public STI::Device::DeviceMessageListener<STI::Device::ChannelUpdateMessage>
{
public:

	TestDeviceChannelUpdateListener() {}

	void handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess)
	{
		for (auto& updates : mess->channelValues) {
			std::cout << "Update channel " << updates.first << " to " << updates.second.print() << std::endl;
		}
	}
};


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	init();

	std::shared_ptr<DeviceMessageReceiver> receiver;
	
	using STI::Device::EngineJobUpdateDeviceMessage;
	using STI::Device::AttributeUpdateMessage;
	using STI::Device::ChannelUpdateMessage;

	//The ID of the device generating the messages
	STI::Device::DeviceID messageSourceID = getID();	//Use the local ID for this example.

	if (getMessageReceiver(receiver)) {

		// Listen to attribute messages using a lambda function
		receiver->addListener<AttributeUpdateMessage>(messageSourceID, "TestDeviceAttributeListener", 
			[](const std::shared_ptr<AttributeUpdateMessage>& mess) {

				for (auto& tuple : mess->attributes) {
					std::cout << "Attribute update: {" << tuple.first << ", " << tuple.second << "}" << std::endl;
				}
			});

		// Listen to engine job messages using a lambda function
		STI::Device::DeviceID serverID("localhost/0/STI Server");		//listen to server

		receiver->addListener<EngineJobUpdateDeviceMessage>(serverID, "TestDeviceJobUpdateListener", 
			[](const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) {
				std::cout << "Engine message: " 
					<< STI::Engine::EngineJobStatusToString(mess->getEngineJob()->getStatus()) << std::endl;
			});
		

		STI::Device::DeviceID testID("localhost/0/TestDevice");		//listen to TestDevice
		addPartner(testID);
		// Listen to channel update messages using a listener class
		auto channelListener = std::make_shared<TestDeviceChannelUpdateListener>();
		receiver->addListener<ChannelUpdateMessage>(testID, "TestDeviceChannelListener", channelListener);
	}
}


void TestDevice::init()
{
	// *** Define channels *** //
	addOutputChannel(0, MixedValueType::Double, "coil current");		//channel 0, must be a double
	addOutputChannel(1, MixedValueType::Int, "temperature setpoint");	//channel 1, must be an integer

	// *** Define attributes *** //
	addAttribute("x", "22");

	addAttribute("TriggerSource", "Hardware", { "Hardware", "Software" })
		.setSetter([this](const std::string& value) -> bool {
			hardwareTrigger = value.compare("Hardware") == 0;
			return true;
		})
		.setRefresher([this]() -> std::string {
			return (hardwareTrigger ? "Hardware" : "Software");	//ensure bool and attribute are consistent
		});
}

bool TestDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	bool success = false;

	switch (channel)
	{
	case 0:
		//coil current (Double)
		std::cout << "Ch:" << channel << ", " << "coil current: " << value.getDouble() << std::endl;
		success = true;
		break;
	case 1:
		//temperature setpoint (Int)
		std::cout << "Ch:" << channel << ", " << "temperature setpoint: " << value.getInt() << std::endl;
		success = true;
		break;
	default:
		break;
	}

	return success;
}