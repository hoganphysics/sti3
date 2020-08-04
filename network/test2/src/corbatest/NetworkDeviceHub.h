#ifndef STI_NETWORK_NETWORKDEVICEHUB_H
#define STI_NETWORK_NETWORKDEVICEHUB_H

#include "DeviceID.h"
#include "Device.h"

#include <memory>

namespace STI
{
namespace Network
{

class LocalDeviceHub;
class NetworkDeviceHubWrapper;

class NetworkDeviceHub
{
public:
	NetworkDeviceHub(const std::string& name);

	bool addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node);

	bool connect(const std::shared_ptr<LocalDeviceHub>& hub);

private:

	std::shared_ptr<LocalDeviceHub> localHub;
	std::shared_ptr<NetworkDeviceHubWrapper> deviceHubWrapper;
};


//
////thin wrapper around LocalHub that also holds the TDeviceHub_i servant of the same Hub
//class NetworkDeviceHubWrapper : public STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>
//{
//
//};
//
////thin wrapper around a LocalDevice that also holds a TDevice_i servant of the same Device
//class NetworkDeviceWrapper : public STI::Device::Device
//{
//	NetworkDeviceWrapper(const std::shared_ptr<STI::Device::Device>& device);
//};
//
//
//
////this is not good... 
//class NetworkDeviceHub
//{
//public:
//
//	//needs
//	LocalCollector<HubID, NetworkDeviceHubWrapper> collector;
//	// or really, it just needs to always wrap before adding to LocalHub.
//
//	//1) gets root reference using OrbManager
//	//2) connects to the hub at the target server (gets TDeviceHub_ptr)
//	//3) Make RemoteDeviceHub and add it to it's LocalHub
//
//	//Is it a LocalHub, or does it have one?
//	//It should have one, and it should also have a TDeviceHub_i which redirects to the LocalHub instance.
//
////	void addDevice(Device& dev);		//for adding local device(s)
////	void run()
////	{
////		Hub<>::connect(local, remote);		//regular connection method, using strored remote reference.
////		orbManager.run();	//blocking
////	}
////
////	std::shared_ptr<LocalDeviceHub> local;
////	std::shared_ptr<RemoteDeviceHub> remote;	//reference received from CORBA nameservice at target IP
//};


} //Network
} //STI


#endif

