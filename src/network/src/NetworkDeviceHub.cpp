
#include "NetworkDeviceHub.h"

#include "NetworkDeviceHubWrapper.h"
#include "LocalDeviceHub.h"
#include "DeviceHub.h"
#include "RemoteDeviceHub.h"
#include "DeviceID.h"

#include "ORBManager.h"

#include <sstream>
#include <iostream>


using STI::Network::NetworkDeviceHub;
using STI::Device::DeviceID;
using STI::Network::LocalDeviceHub;
using STI::Network::ORBManager;
using STI::Network::DeviceHub;


unsigned NetworkDeviceHub::hubNumber = 0;


NetworkDeviceHub::NetworkDeviceHub(const std::string& nameServiceAddress)
	: NetworkDeviceHub(NetworkDeviceHub::nextHubName(), "localhost", 0, nameServiceAddress)
{
	_usingDefaultHubID = true;
}

NetworkDeviceHub::NetworkDeviceHub(const std::string& name, const std::string& address, unsigned short module, const std::string& nameServiceAddress)
{
	_autoReconnectRemoteHubs = true;
	_autoConnect = true;
	_nameServiceAddress = nameServiceAddress;
	_usingDefaultHubID = false;

	localHub = std::make_shared<LocalDeviceHub>(name, address, module);
	deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
}

NetworkDeviceHub::~NetworkDeviceHub()
{
}

std::string NetworkDeviceHub::nextHubName()
{
	//static method to ensure all default hub names are unique
	hubNumber++;
	return (std::string("NetworkDeviceHub_") + STI::Utils::valueToString(hubNumber));
}


void NetworkDeviceHub::setTargetHubs(const std::vector<std::string>& hubIDs)
{ 
	for (auto& id : hubIDs) {
		targetHubs.insert(id);
	}
}

void NetworkDeviceHub::autoReconnectRemoteHubs(bool enabled)
{ 
	_autoReconnectRemoteHubs = enabled;
}

bool NetworkDeviceHub::addNode(const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (node != 0) {
		return addNode(node->getID(), node);
	}
	return false;
}

bool NetworkDeviceHub::addNode(const DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (_usingDefaultHubID && localHub != 0 && localHub->numberOfNodes() == 0) {
		//First attached node, and the NetworkDeviceHub is configured with a default HubID
		//By default, the HubID is set to match the DeviceID of the first added node.
		HubID newHubID(id.getName(), id.getAddress(), id.getModule());
		localHub->setID(newHubID);
	}

	bool success = false;

	if (localHub != 0) {
		success = deviceHubWrapper->addNode(id, node);
	}

	std::string nodeTargetServerID = id.getTargetServerID();

	if (success && _autoConnect && nodeTargetServerID.compare("root") != 0) {
		//add to list of hubs this hub will attempt to connect to
		targetHubs.insert(nodeTargetServerID);
	}

	return success;
}

bool NetworkDeviceHub::connect(const std::shared_ptr<LocalDeviceHub>& hub)
{
	//could just wrap localHub with a NetworkDeviceHubWrapper here so the local hub
	//is transparently on the network.  Would still act local.
	//Problem: hubs connected to other local hubs...
	//**Probably better if we have a walker visitor that can gather Network connection info.

	//No -- it should wrap hub in a NetworkDeviceHubWrapper ! 

	//return LocalDeviceHub::connect(localHub, hub);

	auto localHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(hub);

	return LocalDeviceHub::connect(deviceHubWrapper, localHubWrapper);
}

void NetworkDeviceHub::run(bool block)
{
	if (orbmanager != 0 && orbmanager->running()) {
		return;
	}
	else {
		//orbmanager = std::make_unique<ORBManager>(_nameServiceAddress, "");
		orbmanager = ORBManager::getInstance(_nameServiceAddress, "");
	}

	//All hubs register references in two contexts in the NameService:
	//(1)  STI/<Hub ID>/TDeviceHub.Object
	//(2)  STI/<Target Server ID>/<Hub ID>/TDeviceHub.Object  (for all target hubs, except root)

	//Register this TDeviceHub with NameService
	std::stringstream hubContext;
	STI::TNetwork::TDeviceHub_ptr tDeviceHubLocal;	//var type instead?
	STI::TNetwork::TDeviceHub_ptr tDeviceHubRemote;// = STI::TNetwork::TDeviceHub::_nil();
	CORBA::Object_ptr obj;

	hubContext << "STI" << "/" << localHub->getID().id() << "/" << "TDeviceHub.Object";

	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(deviceHubWrapper, tDeviceHubLocal)) {
		std::cerr
			<< "Error: Unable to get TDeviceHub reference from servant." << std::endl;
		return;
	}

	orbmanager->bindObjectReference(hubContext.str(), tDeviceHubLocal);


	hubContext.str("");
	hubContext.clear();
	hubContext << "STI" << "/" << localHub->getID().id();// << "/";


	//Auto reconnect to Hubs that have connected before
	std::vector<std::string> liveHubs;
	orbmanager->getAllLiveObjectContexts(hubContext.str(), "TDeviceHub.Object", liveHubs);
	std::shared_ptr<RemoteDeviceHub> remoteHub;
	
	hubContext << "/" << "TDeviceHub.Object";
	std::string thisHubContext = hubContext.str();

	//Get TDeviceHub from all live hubs under Hub context; wrap them in RemoteDeviceHubs and reconnect
	for (auto& hubRefContextName : liveHubs) {
		if (hubRefContextName.compare(thisHubContext) != 0 &&	//don't connect to self
			orbmanager->getObjectReference(hubRefContextName, obj)) {
			
			tDeviceHubRemote = STI::TNetwork::TDeviceHub::_narrow(obj);

			if (!CORBA::is_nil(tDeviceHubRemote)) {
				
				remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);

				LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
			}

		}
	}

	//Add reference to this Hub under all target Hubs (for rebind if target Hub restarts)
	for (auto& targetHub : targetHubs) {
		hubContext.str("");
		hubContext.clear();

		hubContext << "STI" << "/" << STI::Utils::replaceChars(targetHub, ".", "_") << "/"
			<< localHub->getID().id() << "/" << "TDeviceHub.Object";

		//Add reference to this Hub under the target hub context (for rebind if target hub restarts)
		orbmanager->bindObjectReference(hubContext.str(), tDeviceHubLocal);
	}

	//Reconnect to target Hubs if they are live
	reconnectToTargetHubs();		//attempt once; make background thread instead?

	orbmanager->run();	//doesn't block

	orbmanager->getAllLiveObjectContexts("STI", "TDeviceHub.Object", liveHubs);

	if (block) {
		orbmanager->block();
	}
}

void NetworkDeviceHub::reconnectToTargetHubs()
{
	std::cout << "Attempting to connect to targets... ";

	unsigned count = 0;

	HubID hubID;

	//Reconnect to target Hubs if they are live
	for (auto& targetHub : targetHubs) {

		if (HubID::stringToHubID(targetHub, hubID) && !localHub->containsHub(hubID)) {
			if (reconnectToTargetHub(targetHub)) { count++; }
		}
	}

	std::cout << "success for " << count << "/" << targetHubs.size() << std::endl;
}

bool NetworkDeviceHub::reconnectToTargetHub(const std::string& targetHub)
{
	bool success = false;

	std::stringstream hubContext;

	STI::TNetwork::TDeviceHub_ptr tDeviceHubRemote; // = STI::TNetwork::TDeviceHub::_nil();
	CORBA::Object_ptr obj;

	std::shared_ptr<RemoteDeviceHub> remoteHub;

	//Context of target hub reference
	hubContext << "STI" << "/" << STI::Utils::replaceChars(targetHub, ".", "_")
		<< "/" << "TDeviceHub.Object";

	//Reconnect to target hub if it is live
	if (orbmanager->getObjectReference(hubContext.str(), obj)) {

		tDeviceHubRemote = STI::TNetwork::TDeviceHub::_narrow(obj);

		if (!CORBA::is_nil(tDeviceHubRemote)) {
			remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);

			success = LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
		}
	}

	return success;
}

void NetworkDeviceHub::reconnectLoop()
{
	std::unique_lock<std::mutex> lck(hubMutex);

	do {
		reconnectToTargetHubs();

	} while (reconnectCondition.wait_for(lck, std::chrono::seconds(1)) == std::cv_status::timeout);
}

void NetworkDeviceHub::walk(LocalDeviceHub::HubNodeWalker& root) const
{
	if (localHub != 0) {
		localHub->walk(root);
	}
}

