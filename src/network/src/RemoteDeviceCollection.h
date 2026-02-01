#ifndef STI_NETWORK_REMOTEDEVICECOLLECTION_H
#define STI_NETWORK_REMOTEDEVICECOLLECTION_H

#include "generated/deviceNet.h"
#include <sti/device/DeviceCollection.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{

class RemoteDeviceCollection : public STI::Device::DeviceCollection,
							   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceCollection>	//mixin
{
public:

	RemoteDeviceCollection(::STI::TNetwork::TDeviceCollection_var deviceCollection);

	bool add(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Device::Device>& node);
	bool remove(const STI::Device::DeviceID& id);

	bool contains(const STI::Device::DeviceID& id) const;
	unsigned size() const;
	
	bool get(const STI::Device::DeviceID& id, std::shared_ptr<STI::Device::Device>& node) const;
	void getIDs(std::set<STI::Device::DeviceID>& ids) const;

	void cleanup();

	void clear();

	bool ping() const;

private:

	mutable std::mutex collectionMutex;

};


} //Network
} //STI


#endif

