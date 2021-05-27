#ifndef STI_NETWORK_REMOTEDEVICEHUB_H
#define STI_NETWORK_REMOTEDEVICEHUB_H

#include "Hub.h"
#include "HubID.h"
#include "Device.h"
#include "DeviceID.h"
#include "HubTrace.h"
#include "DeviceHub.h"
#include "deviceNet.h"
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{


class RemoteDeviceHub : public STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>,
						public STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceHub>	//mixin
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

	const HubID& getID() const;

	void walk(NodeWalker<STI::Device::DeviceID, STI::Device::Device>& root, const HubTrace& trace) const;

private:

	void _getHubID();

	//bool getRemoteDevice(const typename std::shared_ptr<STI::Device::Device>& device, STI::TNetwork::TDevice_ptr& tDevice);

	//::STI::TNetwork::TDeviceHub_var tDeviceHub;		//remote reference
	
	mutable std::mutex hubMutex;

	mutable HubID hubID;


};


} //Network
} //STI


#endif

