#include "JLocalDevice.h"
#include <sti/LocalDevice.h>
#include <sti/device/DeviceMessageReceiver.h>
#include "JDeviceMessageReceiver.h"
#include "JEventEngineScheduler.h"
#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/LocalChannel.h>

#include <memory>


using STI::Device::JLocalDevice;
using STI::Device::JDeviceMessageReceiver;
using STI::Engine::JEventEngineScheduler;
using STI::Engine::EventEngineScheduler;
using STI::Device::LocalChannel;
using STI::Engine::EngineID;
using STI::Device::ChannelType;
using STI::Utils::MixedValueType;
using STI::Device::LocalAttribute;
using STI::Device::DeviceMessage;


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
    }
}

JLocalDevice::~JLocalDevice()
{
}

LocalAttribute& JLocalDevice::addAttribute(const std::string& key, const std::string& initialValue)
{
    return wrappedLocalDevice->addAttribute(key, initialValue);
}

LocalAttribute& JLocalDevice::addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues)
{
    return wrappedLocalDevice->addAttribute(key, initialValue, allowedValues);
}

LocalAttribute& JLocalDevice::addAttribute(const std::string& key, const std::string& initialValue, const std::string& allowedValues)
{
    std::vector<std::string> allowedValuesVec;
    STI::Utils::splitString(allowedValues, ",", allowedValuesVec);
    return wrappedLocalDevice->addAttribute(key, initialValue, allowedValuesVec);
}

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

void JLocalDevice::addEventTarget(const DeviceID& id)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addEventTarget(id);
    }
}


void JLocalDevice::addEventTarget(const DeviceID& id, const std::string& alias)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addEventTarget(id, alias);
    }
}

void JLocalDevice::addPartner(const DeviceID& id)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addPartner(id);
    }
}


void JLocalDevice::addPartner(const DeviceID& id, const std::string& alias)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addPartner(id, alias);
    }
}


void JLocalDevice::addTask(const std::shared_ptr<STI::Utils::Task>& task)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addTask(task);
    }
}

std::shared_ptr<STI::Utils::FileServer> JLocalDevice::getFileServer()
{
    std::shared_ptr<STI::Utils::FileServer> fileServer;
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->getFileServer(fileServer);
    }
    return fileServer;
}

std::shared_ptr<STI::Device::JDeviceMessageReceiver> JLocalDevice::getMessageReceiver()
{
    return jReceiver;
}

void JLocalDevice::sendMessage(const std::shared_ptr<DeviceMessage>& mess)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->sendMessage(mess);
    }
}

void JLocalDevice::addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener)
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->addCollectionListener(listener);
    }
}

bool JLocalDevice::write(int channel, const STI::Utils::MixedValue& value)
{
    if (wrappedLocalDevice != 0) {
        return wrappedLocalDevice->write(static_cast<short>(channel), value);
    }
    return false;
}

//can be overridden in Java
bool JLocalDevice::writeChannel(int channel, const STI::Utils::MixedValue& value)
{
    if (wrappedLocalDevice != 0) {
        return wrappedLocalDevice->writeChannelDefault(static_cast<short>(channel), value);
    }
    return false;
}


STI::Utils::MixedValue JLocalDevice::read(int channel, const STI::Utils::MixedValue& value)
{
    if (wrappedLocalDevice != 0) {
        STI::Utils::MixedValue data;
        wrappedLocalDevice->read(static_cast<short>(channel), value, data);
        return data;
    }

    STI::Utils::MixedValue empty;
    return empty;
}

//can be overridden in Java
STI::Utils::MixedValue JLocalDevice::readChannel(int channel, const STI::Utils::MixedValue& value)
{
    if (wrappedLocalDevice != 0) {
        STI::Utils::MixedValue data;
        wrappedLocalDevice->readChannelDefault(static_cast<short>(channel), value, data);
        return data;
    }

    STI::Utils::MixedValue empty;
    return empty;
}


void JLocalDevice::stopRW()
{
    if (wrappedLocalDevice != 0) {
        wrappedLocalDevice->stopRW();
    }
}



//////////// LocalDeviceProxy //////////////

void JLocalDevice::LocalDeviceProxy::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    std::vector<std::shared_ptr<STI::Engine::SynchronousEventAdapter>> synchedEventAdapters;

    if (jLocalDevice != 0) {

        jLocalDevice->parseEvents(events, synchedEventAdapters);

        synchedEvents.reserve(synchedEventAdapters.size());

        synchedEvents.insert(synchedEvents.end(), 
                             std::make_move_iterator(synchedEventAdapters.begin()), 
                             std::make_move_iterator(synchedEventAdapters.end()));
        synchedEventAdapters.erase(synchedEventAdapters.begin(), synchedEventAdapters.end());
    }
}


bool JLocalDevice::LocalDeviceProxy::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
    if (jLocalDevice == 0) return false;

    return jLocalDevice->writeChannel(static_cast<int>(channel), value);
}

bool JLocalDevice::LocalDeviceProxy::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
    if (jLocalDevice == 0) return false;

    STI::Utils::MixedValue result = jLocalDevice->readChannel(static_cast<int>(channel), value);
    data = result;
    return true;
}
