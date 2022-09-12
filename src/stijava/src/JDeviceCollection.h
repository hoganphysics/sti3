#ifndef STI_DEVICE_JDEVICECOLLECTION_H
#define STI_DEVICE_JDEVICECOLLECTION_H

#include <sti/device/DeviceCollection.h>

#include <memory>
#include <set>

namespace STI
{
namespace Device
{

class DeviceID;
class JDevice;

//Java DeviceCollection wrapper
class JDeviceCollection : public STI::Device::DeviceCollection
{
public:
	
	JDeviceCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	~JDeviceCollection();

    bool add(const DeviceID& id, const std::shared_ptr<JDevice>& node);
	std::shared_ptr<JDevice> get(const DeviceID& id) const;
	std::set<DeviceID>& getIDs();

    //Collection
	bool remove(const DeviceID& id);
	bool contains(const DeviceID& id) const;
	unsigned size() const;
	void cleanup();
	void clear();

private:

    //Collection
    bool add(const DeviceID& id, const DeviceCollection::T_ptr& node);
	bool get(const DeviceID& id, DeviceCollection::T_ptr& node) const;
	void getIDs(std::set<DeviceID>& ids) const;

    std::shared_ptr<DeviceCollection> localDeviceCollection;
    std::set<DeviceID> ids; //local copy, so we can return by reference.
};

} //Device
} //STI

#endif
