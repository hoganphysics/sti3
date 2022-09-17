
#include "JDeviceCollection.h"
#include <sti/device/DeviceID.h>
#include "JDevice.h"
#include <sti/device/Device.h>

#include <memory>
#include <iostream>

using STI::Device::JDeviceCollection;
using STI::Device::DeviceID;
using STI::Device::JDevice;


JDeviceCollection::JDeviceCollection(const std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
    localDeviceCollection = collection;
}

JDeviceCollection::~JDeviceCollection()
{
}

bool JDeviceCollection::add(const DeviceID& id, const std::shared_ptr<JDevice>& node)
{
    return (localDeviceCollection != 0 && 
    localDeviceCollection->add(id, std::static_pointer_cast<STI::Device::Device>(node))
    );
}

std::shared_ptr<JDevice> JDeviceCollection::get(const DeviceID& id) const
{
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<JDevice> jDevice;
    
    // std::cout << "**** JDeviceCollection::get " 
    // << ((localDeviceCollection != 0) ? "1" : "0") << "," 
    // << ((localDeviceCollection->get(id, device)) ? "1" : "0") << "," 
    // << ((device != 0) ? "1" : "0") 
    // << std::endl;

    // std::cout << "**** size=" << localDeviceCollection->size() << std::endl;
    // std::set<DeviceID> ids;
    // localDeviceCollection->getIDs(ids);
    // for (auto& i : ids) {
    //     std::cout << "**** id=" << i.getID() << " : " << i.getTargetServerID() << std::endl;
    // }

    
    if (localDeviceCollection != 0 && localDeviceCollection->get(id, device) && device != 0) {
        // std::cout << "**** JDeviceCollection::get inside if" << std::endl;
        jDevice = std::make_shared<JDevice>(device);
    }
    return jDevice;
}

std::set<DeviceID> JDeviceCollection::getIDs()
{
    std::set<DeviceID> ids;

    if (localDeviceCollection != 0) {
        localDeviceCollection->getIDs(ids);
    }
    return ids;
}

bool JDeviceCollection::remove(const DeviceID& id)
{
    return (localDeviceCollection != 0 && localDeviceCollection->remove(id));
}

bool JDeviceCollection::contains(const DeviceID& id) const
{
    return (localDeviceCollection != 0 && localDeviceCollection->contains(id));
}

unsigned JDeviceCollection::size() const
{
    if (localDeviceCollection != 0) {
        return localDeviceCollection->size();
    }
    return 0;
}

void JDeviceCollection::cleanup()
{
    if (localDeviceCollection != 0) {
        return localDeviceCollection->cleanup();
    }
}

void JDeviceCollection::clear()
{
    if (localDeviceCollection != 0) {
        return localDeviceCollection->clear();
    }
}


bool JDeviceCollection::add(const DeviceID& id, const DeviceCollection::T_ptr& node)
{
    return (localDeviceCollection != 0 && localDeviceCollection->add(id, node));
}

bool JDeviceCollection::get(const DeviceID& id, DeviceCollection::T_ptr& node) const
{
    return (localDeviceCollection != 0 && localDeviceCollection->get(id, node));
}

void JDeviceCollection::getIDs(std::set<DeviceID>& ids) const
{
    if (localDeviceCollection != 0) {
        return localDeviceCollection->getIDs(ids);
    }
}
