
#include "TestDevice.h"

#include <iostream>
#include <string>


TestDevice::TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
: STI::Device::LocalDevice(name, address, module, targetServer)
{
    addChannel(1, 
               STI::Device::ChannelType::Output, 
               STI::Utils::MixedValueType::Empty, 
               STI::Utils::MixedValueType::Double, 
               "test channel");

    STI::Engine::EngineID id(0);
    addEventEngine(id);
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
