#ifndef STI_NETWORK_REMOTEDEVICEHUB_H
#define STI_NETWORK_REMOTEDEVICEHUB_H

#include "Hub.h"
#include "HubID.h"
#include "Device.h"
#include "DeviceID.h"
#include "HubTrace.h"
#include "DeviceHub.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{


//ACTUALLY this doesn't work, since the Hub can't resolve the child class.  Not possible to know when
//to send a servant reference with this approach...

//thin wrapper around a LocalDevice that also holds a TDevice_i servant of the same Device
class RemoteDeviceHub : public STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>
{
public:

	RemoteDeviceHub(::STI::TNetwork::TDeviceHub_ptr deviceHub);

//	void getNodeIDs(std::set<STI::Device::DeviceID>& ids) const;
//	void getHubIDs(std::set<HubID>& ids) const;

	bool addHub(const HubID& id, const std::shared_ptr<DeviceHub>& hub);
	bool removeHub(const HubID& id);

	bool removeNode(const STI::Device::DeviceID& id, const HubTrace& trace);
	bool refresh(const HubTrace& trace);		//local and remote	

	bool distribute(const STI::Device::DeviceID& id, 
		const typename std::shared_ptr<STI::Device::Device>& node,
		const HubTrace& trace, const HubID& first);	//remote add (called by other hubs offering a reference); trail tracked
	bool distributeNodes(const HubID& targetHub); //, const HubTrace& trace);		//distribute all local nodes to target hub

															  //Force redistribution of all Nodes owned by this Hub to all connected Hubs.
	bool redistributeNodes(const HubTrace& trace);		//distribute all owned Nodes to all connected Hubs

	const HubID& getID();

private:

	//bool getRemoteDevice(const typename std::shared_ptr<STI::Device::Device>& device, STI::TNetwork::TDevice_ptr& tDevice);

	::STI::TNetwork::TDeviceHub_var tDeviceHub;		//remote reference
	HubID hubID;

};


} //Network
} //STI


#endif

