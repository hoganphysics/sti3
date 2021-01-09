#ifndef STI_NETWORK_NETWORKDEVICEHUB_H
#define STI_NETWORK_NETWORKDEVICEHUB_H

#include "DeviceID.h"
#include "Device.h"
#include "DeviceHub.h"
#include "LocalDeviceHub.h"

#include <memory>
#include <set>
#include <mutex>
#include <condition_variable>

namespace STI
{
namespace Network
{

class LocalDeviceHub;
class NetworkDeviceHubWrapper;
class ORBManager;


class NetworkDeviceHub
{
public:

	NetworkDeviceHub(const std::string& nameServiceAddress);
	NetworkDeviceHub(const std::string& name, const std::string& address, unsigned short module, const std::string& nameServiceAddress);
	~NetworkDeviceHub();
	
	bool addDevice(const typename std::shared_ptr<STI::Device::Device>& node);
	bool connect(const std::shared_ptr<LocalDeviceHub>& hub);

	void setTargetHubs(const std::vector<std::string>& hubIDs);

	//options
	void autoConnectToTargetServers(bool enabled) { _autoConnect = enabled; }
	void autoReconnectRemoteHubs(bool enabled);
	void setNameServiceAddress(const std::string& nameServiceAddress) { _nameServiceAddress = nameServiceAddress; }

	void run(bool block = true);

	void walk(LocalDeviceHub::HubNodeWalker& root) const;

private:

	bool addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node);

	void reconnectLoop();
	void reconnectToTargetHubs();
	bool reconnectToTargetHub(const std::string& targetHub);

	std::shared_ptr<LocalDeviceHub> localHub;
	std::shared_ptr<NetworkDeviceHubWrapper> deviceHubWrapper;

	std::set<std::string> targetHubs;	//std::set so they are unique (only one copy of each)

	bool _autoReconnectRemoteHubs;
	bool _autoConnect;
	std::string _nameServiceAddress;
	bool _usingDefaultHubID;
	
	//makes sure default names are unique
	static std::string nextHubName();
	static unsigned hubNumber;

	mutable std::mutex hubMutex;
	mutable std::condition_variable reconnectCondition;

	std::shared_ptr<ORBManager> orbmanager;		//ptr here to avoid adding ORBManager header to public API
};


} //Network
} //STI


#endif

