#ifndef STI_NETWORK_REMOTEDEVICECOLLECTION_H
#define STI_NETWORK_REMOTEDEVICECOLLECTION_H

#include "deviceNet.h"
#include "DeviceCollection.h"
#include "Device.h"
#include "DeviceID.h"

#include <memory>

namespace STI
{
namespace Network
{

class RemoteDeviceCollection : public STI::Device::DeviceCollection
{
public:

	RemoteDeviceCollection(::STI::TNetwork::TDeviceCollection_ptr deviceCollection);

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

	::STI::TNetwork::TDeviceCollection_var tDeviceCollection;		//remote reference

};


} //Network
} //STI


#endif

