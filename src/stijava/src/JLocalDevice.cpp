
#include "JLocalDevice.h"
#include <sti/LocalDevice.h>
#include <sti/device/DeviceMessageReceiver.h>
#include "JDeviceMessageReceiver.h"
#include "JEventEngineScheduler.h"
#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/LocalChannel.h>

#include <memory>

#include <iostream>

using STI::Device::JLocalDevice;
using STI::Device::JDeviceMessageReceiver;
using STI::Engine::JEventEngineScheduler;
using STI::Engine::EventEngineScheduler;
using STI::Device::LocalChannel;
using STI::Engine::EngineID;
using STI::Device::ChannelType;
using STI::Utils::MixedValueType;



JLocalDevice::JLocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
//        : STI::Device::JDevice(name, address, module, targetServer)
        : STI::Device::JDevice( std::make_shared<LocalDeviceProxy>(this, name, address, module, targetServer) )
{
    //Slight hack here. We want JLocalDevice to inherit from JDevice AND to delegate to the same
    //wrapped pointer. Using the local constructor, the pointer is created as a LocalDevice but 
    //stored in JDevice as a Device so the JDevice class can also wrap RemoteDevices. So here we 
    //dynamic_cast back...
    //Note JDevice::wrappedDevice was just created as a LocalDevice, so this is guaranteed to work.

    wrappedLocalDevice = std::dynamic_pointer_cast<STI::Device::LocalDevice>(wrappedDevice);

    if (wrappedLocalDevice != 0) {
        
        //Get and store DeviceMessageReceiver reference
        std::shared_ptr<DeviceMessageReceiver> receiver;
        wrappedLocalDevice->getMessageReceiver(receiver);

        jReceiver = std::make_shared<JDeviceMessageReceiver>(receiver);


        // std::shared_ptr<EventEngineScheduler> scheduler;
        // wrappedLocalDevice->getEngineScheduler(scheduler);
        // jScheduler = std::make_shared<JEventEngineScheduler>(scheduler);        
    }

}

JLocalDevice::~JLocalDevice()
{
}

// void JLocalDevice::test()
// {
//     std::cout << "JLocalDevice::test()" << std::endl;
// }

LocalChannel& JLocalDevice::addChannel(int channelNumber, ChannelType type,
		MixedValueType inputType, MixedValueType outputType, const std::string& defaultName)
{
    return wrappedLocalDevice->addChannel(static_cast<unsigned short>(channelNumber), type, inputType, outputType, defaultName);
}

void JLocalDevice::addEventEngine(const EngineID& engineID)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addEventEngine(engineID);
    }
}

void JLocalDevice::addPartner(const DeviceID& id)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addPartner(id);
    }
}

std::shared_ptr<STI::Device::JDeviceMessageReceiver> JLocalDevice::getMessageReceiver()
{
    return jReceiver;
}

// std::shared_ptr<STI::Device::JDeviceMessageReceiver> JLocalDevice::getEventReceiver2()
// {
//     return jReceiver;
// }

// std::shared_ptr<JEventEngineScheduler> JLocalDevice::getEngineScheduler()
// {
//     return jScheduler;
// }



//TEMP

JLocalDevice::LocalDeviceProxy::TestEvent::TestEvent(const STI::Engine::RawEvent& evt) 
: STI::Engine::SynchronousEventAdapter(evt.time()), evt(evt) 
{
}

void JLocalDevice::LocalDeviceProxy::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    std::cout << "Parsing: " << getID().getName() << std::endl;
    std::cout << "Event count: " << events.size() << std::endl;
    
    if (events.size() > 0) {
        std::cout << events.begin()->second.at(0).print() << std::endl;
        auto evt = std::make_unique<JLocalDevice::LocalDeviceProxy::TestEvent>(events.begin()->second.at(0));
        synchedEvents.push_back(std::move(evt));
    }

    if (jLocalDevice != 0) {
        jLocalDevice->parseEvents(0);	//temp
    }
}


void JLocalDevice::LocalDeviceProxy::TestEvent::playEvent()
{
	std::cout << "Play: " << evt.print() << std::endl;
}