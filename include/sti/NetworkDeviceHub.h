#ifndef STI_NETWORK_NETWORKDEVICEHUB_H
#define STI_NETWORK_NETWORKDEVICEHUB_H

#include <sti/LocalDeviceHub.h>

#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include <sti/network/DeviceHub.h>
#include <sti/utils/Configuration.h>
#include <sti/fwd/TaskScheduler_fwd.h>

#include <condition_variable>
#include <mutex>
#include <memory>
#include <set>
#include <iosfwd>


namespace STI
{
namespace Network
{

class LocalDeviceHub;
class NetworkDeviceHubWrapper;
class ORBManager;
class RemoteDeviceHub;


class NetworkDeviceHub
{
public:

	NetworkDeviceHub(const std::string& nameServiceAddress);
	NetworkDeviceHub(const std::string& nameServiceAddress, const STI::Utils::Configuration& config);	
	NetworkDeviceHub(const STI::Utils::Configuration& config);

	NetworkDeviceHub(const HubID& hubID, const std::string& nameServiceAddress);
	NetworkDeviceHub(const HubID& hubID, const std::string& nameServiceAddress, const STI::Utils::Configuration& config);
	NetworkDeviceHub(const HubID& hubID, const STI::Utils::Configuration& config);

	~NetworkDeviceHub();
	
	bool addDevice(const typename std::shared_ptr<STI::Device::Device>& node);
	bool addDevice(const typename std::shared_ptr<STI::Device::Device>& node, const HubID& hubID);
	bool addDevice(const typename std::shared_ptr<STI::Device::Device>& node, const std::string& serverHubID);
	
	bool connect(const std::shared_ptr<LocalDeviceHub>& hub);

	HubID getID() const;

	void getDeviceIDs(std::set<STI::Device::DeviceID>& ids) const;

	void setTargetHubs(const std::vector<HubID>& hubIDs);
	void setTargetHubs(const std::vector<std::string>& hubIDs);

	void addTargetHub(const HubID& hubID);
	void addTargetHub(const std::string& hubID);

	//Find the hubID that deviceID is attached to
	bool findHub(const STI::Device::DeviceID& deviceID, HubID& hubID);

	//options
	void setNameServiceAddress(const std::string& nameServiceAddress) { _nameServiceAddress = nameServiceAddress; }

	void run(bool block = true);
	void shutdown();
	void disconnect();

	void walk(LocalDeviceHub::HubNodeWalker& root) const;

	std::string printNetwork();
	std::string printNetwork(const std::string& baseContext);
	
	static std::string printNetwork(const std::string& nameServiceAddress, const std::string& baseContext);

	struct PersistenceOptions
	{
		bool bindToRootContext;
		bool bindToTargetContexts;
	};

	PersistenceOptions& getPersistenceOptions();

private:

	void unblock();
	
	bool addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node);

	void connectToTargetHubs();
	
	bool registerHubContext();
	bool unregisterHubContext();
	void refreshHubContext();

	std::string makeHubContextPath(const std::string& baseContext, const HubID& hubID);
	std::string makeHubContext(const std::string& baseContext, const HubID& hubID);
	
	bool getRemoteHub(const std::string& remoteHubContext, std::shared_ptr<RemoteDeviceHub>& remoteHub, std::ostream& errorBuf);
	bool connectRemoteHub(const std::string& remoteHubContext);

	PersistenceOptions persistence;

	std::shared_ptr<LocalDeviceHub> localHub;
	std::shared_ptr<NetworkDeviceHubWrapper> deviceHubWrapper;

	std::shared_ptr<STI::Utils::TaskScheduler> refreshScheduler;

	std::set<HubID> targetHubs;	//std::set so they are unique (only one copy of each)

	std::string _nameServiceAddress;
	bool _usingDefaultHubID;
	bool useAutoTargetHubIDs;
	
	//makes sure default names are unique
	static std::string nextHubName();
	static unsigned hubNumber;

	std::string stiContext;
	std::string hubObjectName;
	std::string hubIDprefix;

	std::string thisHubContext;
	std::string hubContextPath;

	mutable std::mutex hubMutex;
	mutable std::condition_variable reconnectCondition;

	std::shared_ptr<ORBManager> orbmanager;		//ptr here to avoid adding ORBManager header to public API
};


} //Network
} //STI


#endif

