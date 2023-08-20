
#include "DeviceCollectionPy.h"
#include "DevicePy.h"

#include <sti/device/DeviceID.h>

using STI::Python::DeviceCollectionPy;


DeviceCollectionPy::DeviceCollectionPy(const std::shared_ptr<STI::Device::DeviceCollection>& collection)
: deviceCollection(collection)
{
}

DeviceCollectionPy::~DeviceCollectionPy()
{
}


bool DeviceCollectionPy::add(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Python::DevicePy>& node)
{
    if (deviceCollection != 0 && node != 0) {
        return deviceCollection->add(id, node->getDevice());
    }
    return false;
}

bool DeviceCollectionPy::remove(const STI::Device::DeviceID& id)
{
    if (deviceCollection != 0) {
        return deviceCollection->remove(id);
    }
    return false;
}

bool DeviceCollectionPy::contains(const STI::Device::DeviceID& id) const
{
    if (deviceCollection != 0) {
        return deviceCollection->contains(id);
    }
    return false;
}

unsigned DeviceCollectionPy::size() const
{
    if (deviceCollection != 0) {
        return deviceCollection->size();
    }
    return 0;
}


std::shared_ptr<STI::Python::DevicePy> DeviceCollectionPy::get(const STI::Device::DeviceID& id) const
{
    std::shared_ptr<STI::Device::Device> device;

    if (deviceCollection != 0) {
        deviceCollection->get(id, device);
    }

    auto devicePy = std::make_shared<STI::Python::DevicePy>(device);

    return devicePy;
}

std::vector<STI::Device::DeviceID> DeviceCollectionPy::getIDs() const
{
    std::vector<STI::Device::DeviceID> idsVec;

    std::set<STI::Device::DeviceID> ids;

    if (deviceCollection != 0) {
        deviceCollection->getIDs(ids);

        for(auto& id : ids) {
            idsVec.push_back(id);
        }
    }
    return idsVec;
}

void DeviceCollectionPy::clear()
{
    if (deviceCollection != 0) {
        return deviceCollection->clear();
    }
}
