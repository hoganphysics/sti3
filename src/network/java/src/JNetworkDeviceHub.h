#ifndef STI_NETWORK_JNETWORKDEVICEHUB_H
#define STI_NETWORK_JNETWORKDEVICEHUB_H

#include "DeviceID.h"
#include "JDevice.h"

#include <memory>
#include <string>

namespace STI
{
namespace Network
{

class NetworkDeviceHub;
class JNodeWalker;

class JNetworkDeviceHub
{
public:

	JNetworkDeviceHub(const std::string& nameServiceAddress);
	JNetworkDeviceHub(const std::string& name, const std::string& address, unsigned short module, const std::string& nameServiceAddress);
	~JNetworkDeviceHub();

	bool addNode(const typename std::shared_ptr<STI::Device::JDevice>& node);

	//bool connect(const std::shared_ptr<LocalDeviceHub>& hub);
	//void setTargetHubs(const std::vector<std::string>& hubIDs);

	//options
	//void autoConnectToTargetServers(bool enabled) { _autoConnect = enabled; }
	//void autoReconnectRemoteHubs(bool enabled);
	//void setNameServiceAddress(const std::string& nameServiceAddress) { _nameServiceAddress = nameServiceAddress; }

    void run();
	void run(bool block);

    JNodeWalker walk() const;
	//void walk(LocalDeviceHub::HubNodeWalker& root) const;

private:

	std::shared_ptr<NetworkDeviceHub> networkHub;
	
};


} //Network
} //STI


#endif

