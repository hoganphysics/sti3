
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
	_autoConnect = true;
	_nameServiceAddress = nameServiceAddress;
	_usingDefaultHubID = false;

	localHub = std::make_shared<LocalDeviceHub>(name, address, module);
	deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
	
	stiContext = "STI";
	hubObjectName = "TDeviceHub.Object";

	refreshHubContext();
}

NetworkDeviceHub::~NetworkDeviceHub()
{
}

void NetworkDeviceHub::refreshHubContext()
{
	thisHubContext = makeHubContext(stiContext, localHub->getID().getID());
	hubContextPath = makeHubContextPath(stiContext, localHub->getID().getID());
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


bool NetworkDeviceHub::addDevice(const typename std::shared_ptr<STI::Device::Device>& node)
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
		refreshHubContext();
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
	auto localHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(hub);

	return LocalDeviceHub::connect(deviceHubWrapper, localHubWrapper);
}

bool NetworkDeviceHub::unregisterHubContext()
{
	return false;
}

bool NetworkDeviceHub::registerHubContext()
{
	// Register this TDeviceHub reference in two contexts in the NameService:
	// (1)  STI/<Hub ID>/TDeviceHub.Object
	// (2)  STI/<Target Server ID>/<Hub ID>/TDeviceHub.Object  (for all target hubs, except root)

	bool success = false;
	STI::TNetwork::TDeviceHub_ptr tDeviceHubLocal;

	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(deviceHubWrapper, tDeviceHubLocal)) {
		std::cerr
			<< "Error: Unable to get TDeviceHub reference from servant." << std::endl;
		return false;
	}

	// (1) Bind primary reference to this Hub under root context
	success = orbmanager->bindObjectReference(thisHubContext, tDeviceHubLocal);

	// (2) Bind reference to this Hub under all target Hub contexts (for reconnect when target Hub restarts)
	for (auto& targetHub : targetHubs) {

		std::string targetHubPath = makeHubContextPath(stiContext, targetHub);

		//Add reference to this Hub under the target hub context (for rebind if target hub restarts)
		success &= orbmanager->bindObjectReference( 
			makeHubContext(targetHubPath, localHub->getID().getID()), 
			tDeviceHubLocal);
	}

	return success;
}


void NetworkDeviceHub::run(bool block)
{
	if (orbmanager != 0 && orbmanager->running()) {
		return;
	}
	else {
		orbmanager = ORBManager::getInstance(_nameServiceAddress, "");
	}

	if (!registerHubContext()) {
		return;
	}

	// Find live registered Hubs that attempted to connect to this Hub. Attempt to connect.
	std::vector<std::string> liveHubs;
	orbmanager->getAllLiveObjectContexts(hubContextPath, hubObjectName, liveHubs);

	for (auto& hubContext : liveHubs) {
		if (hubContext.compare(thisHubContext) != 0) {		//don't connect to self
			connectRemoteHub(hubContext);
		}
	}

	// Attempt to connect to target Hubs of this Hub
	connectToTargetHubs();

	//Start ORB (network servants go live)
	orbmanager->run();	//doesn't block

	if (block) {
		orbmanager->block();
	}
}

void NetworkDeviceHub::connectToTargetHubs()
{
	HubID hubID;

	//Reconnect to target Hubs if they are live
	for (auto& targetHubID : targetHubs) {

		if (HubID::stringToHubID(targetHubID, hubID) && !localHub->containsHub(hubID)) {
			connectRemoteHub( makeHubContext(stiContext, targetHubID) );
		}
	}
}

std::string NetworkDeviceHub::makeHubContextPath(const std::string& baseContext, const std::string& hubID)
{
	std::stringstream hubContext;
	
	hubContext << baseContext << "/" << STI::Utils::replaceChars(hubID, ".", "_");

	return hubContext.str();
}


std::string NetworkDeviceHub::makeHubContext(const std::string& baseContext, const std::string& hubID)
{
	std::stringstream hubContext;

	hubContext << makeHubContextPath(baseContext, hubID) << "/" << hubObjectName;

	return hubContext.str();
}

bool NetworkDeviceHub::connectRemoteHub(const std::string& remoteHubContext)
{
	bool success = false;

	std::shared_ptr<RemoteDeviceHub> remoteHub;
	STI::TNetwork::TDeviceHub_ptr tDeviceHubRemote; // = STI::TNetwork::TDeviceHub::_nil();
	CORBA::Object_ptr obj;

	//Attempt to find and connect to live hub with remoteHubContext in NameService
	if (orbmanager->getObjectReference(remoteHubContext, obj)) {

		tDeviceHubRemote = STI::TNetwork::TDeviceHub::_narrow(obj);

		if (!CORBA::is_nil(tDeviceHubRemote)) {
			remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);

			success = LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
		}
	}

	return success;
}

void NetworkDeviceHub::walk(LocalDeviceHub::HubNodeWalker& root) const
{
	if (localHub != 0) {
		localHub->walk(root);
	}
}

