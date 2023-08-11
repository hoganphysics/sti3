#include <sti/NetworkDeviceHub.h>
#include <sti/LocalDeviceHub.h>
#include <sti/device/DeviceID.h>
#include <sti/network/DeviceHub.h>

#include "NetworkDeviceHubWrapper.h"
#include "ORBManager.h"
#include "RemoteDeviceHub.h"

#include <sstream>
#include <iostream>
#include <algorithm>

using STI::Network::NetworkDeviceHub;
using STI::Device::DeviceID;
using STI::Network::LocalDeviceHub;
using STI::Network::ORBManager;
using STI::Network::DeviceHub;


unsigned NetworkDeviceHub::hubNumber = 0;


NetworkDeviceHub::NetworkDeviceHub(const std::string& nameServiceAddress)
: NetworkDeviceHub(
	HubID(NetworkDeviceHub::nextHubName(), "localhost", 0),
	STI::Utils::Configuration().set("NetworkHub", "NameService", nameServiceAddress))
{
	_usingDefaultHubID = true;
}

NetworkDeviceHub::NetworkDeviceHub(const std::string& nameServiceAddress, const STI::Utils::Configuration& config)
: NetworkDeviceHub(
	STI::Utils::Configuration().append(config)
		.set("NetworkHub", "NameService", nameServiceAddress)
		)
{
}

NetworkDeviceHub::NetworkDeviceHub(const STI::Utils::Configuration& config)
: NetworkDeviceHub(
	HubID(
		config.get<std::string>("NetworkHub", "HubName", NetworkDeviceHub::nextHubName()),
		config.get<std::string>("NetworkHub", "HubAddress", "localhost"),
		config.get<unsigned short>("NetworkHub", "HubModule", 0)
		),
	config)
{
	_usingDefaultHubID = !(
			config.includes("NetworkHub", "HubName") 
			&& config.includes("NetworkHub", "HubAddress") 
			&& config.includes("NetworkHub", "HubModule")
		);
}

NetworkDeviceHub::NetworkDeviceHub(const HubID& hubID, const std::string& nameServiceAddress)
: NetworkDeviceHub(hubID, nameServiceAddress, STI::Utils::Configuration())
{
}

NetworkDeviceHub::NetworkDeviceHub(const HubID& hubID, const std::string& nameServiceAddress, const STI::Utils::Configuration& config)
: NetworkDeviceHub(hubID, STI::Utils::Configuration().append(config).set("NetworkHub", "NameService", nameServiceAddress))
{
}

NetworkDeviceHub::NetworkDeviceHub(const HubID& hubID, const STI::Utils::Configuration& config)
{
	// _autoConnect = true;
	
	_nameServiceAddress = config.get<std::string>("NetworkHub", "NameService", "localhost:2809" /*default*/ );
	_usingDefaultHubID = false;
	useAutoTargetHubIDs = true;		//attempt to auto connect to hubs with HubIDs derived from attached devices targetServerID

	
	stiContext = "STI";
	hubObjectName = "TDeviceHub.Object";
	hubIDprefix = "Hub::";

	persistence.bindToRootContext = true;
	persistence.bindToTargetContexts = true;


	// STI::Utils::Configuration omniConfig(config.getParameters("omniORB"));
	STI::Utils::Configuration omniConfig;
	omniConfig.set("InitRef", "NameService=corbaname::" + _nameServiceAddress);

	//Add parameters from config file (overwrites any duplicate entries)
	omniConfig.append(STI::Utils::Configuration().append( config.getParameters("omniORB") ));

	orbmanager = STI::Network::ORBManager::getInstance(omniConfig, "");

	localHub = std::make_shared<LocalDeviceHub>(hubID);
	
	if (orbmanager->initialized()) {
		deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
	}

	refreshHubContext();
}

NetworkDeviceHub::~NetworkDeviceHub()
{
	if (localHub != 0) {
		localHub->clear();
	}
}

std::string NetworkDeviceHub::printNetwork()
{
	return printNetwork(stiContext);
}

std::string NetworkDeviceHub::printNetwork(const std::string& baseContext)
{
	std::string network;

	if (orbmanager != 0) {
		network = orbmanager->printNameTree(baseContext);
	}
	return network;
}

//static
std::string NetworkDeviceHub::printNetwork(const std::string& nameServiceAddress, const std::string& baseContext)
{
	// auto orbmanager = STI::Network::ORBManager::getInstance(nameServiceAddress, "");

	// if (orbmanager != 0) {
	// 	return orbmanager->printNameTree(baseContext);
	// }
	
	return "";
}

NetworkDeviceHub::PersistenceOptions& NetworkDeviceHub::getPersistenceOptions()
{
	return persistence;
}

void NetworkDeviceHub::refreshHubContext()
{
	thisHubContext = makeHubContext(stiContext, localHub->getID());
	hubContextPath = makeHubContextPath(stiContext, localHub->getID());
}

std::string NetworkDeviceHub::nextHubName()
{
	//static method to ensure all default hub names are unique
	hubNumber++;
	return (std::string("NetworkDeviceHub_") + STI::Utils::valueToString(hubNumber));
}


void NetworkDeviceHub::setTargetHubs(const std::vector<std::string>& hubIDs)
{ 
	std::vector<HubID> hids;

	HubID hid;
	for (auto& id : hubIDs) {
		if (HubID::stringToHubID(id, hid)) {
			hids.push_back(hid);
		}
	}
	
	setTargetHubs(hids);
}

void NetworkDeviceHub::setTargetHubs(const std::vector<HubID>& hubIDs)
{
	if (hubIDs.size() == 0) return;		//must specify at least one target hub

	targetHubs.clear();
	useAutoTargetHubIDs = false;		//use specified hubs instead

	for (auto& id : hubIDs) {
		addTargetHub(id);
	}
}

void NetworkDeviceHub::addTargetHub(const HubID& hubID)
{
	targetHubs.insert(hubID);
}

void NetworkDeviceHub::addTargetHub(const std::string& hubID)
{
	HubID hid;
	
	if (HubID::stringToHubID(hubID, hid)) {
		addTargetHub(hid);
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
		//By default, the HubID is set to derive from the DeviceID of the first added node,
		//with the hubIDprefix prepended to the device name as follows:
		// DeviceID = localhost/0/dev1
		// HubID = localhost/0/Hub::dev1
		
		HubID newHubID(hubIDprefix + id.getName(), id.getAddress(), id.getModule());
		localHub->setID(newHubID);
		refreshHubContext();
	}

	bool success = false;

	if (localHub != 0 && deviceHubWrapper != 0) {
		success = deviceHubWrapper->addNode(id, node);
	}

	std::string nodeTargetServerID = id.getTargetServerID();
	
	if (success && useAutoTargetHubIDs && nodeTargetServerID.compare("root") != 0) {
		//add to list of hubs this hub will attempt to connect to
		addTargetHub(nodeTargetServerID, hubIDprefix);
	}

	return success;
}

void NetworkDeviceHub::addTargetHub(const std::string& hubID, const std::string& namePrefix)
{
	HubID hid;

	if (HubID::stringToHubID(hubID, hid)) {

		hid.name = namePrefix + hid.name;	//Example: Hub::<name>

		addTargetHub(hid);
	}
}

bool NetworkDeviceHub::findHub(const STI::Device::DeviceID& deviceID, HubID& hubID)
{
	bool found = false;
	std::shared_ptr<RemoteDeviceHub> remoteHub;

	//try hint HubID first, if it is live
	auto inputHubID = makeHubContext(stiContext, hubID);

	if (getRemoteHub(inputHubID, remoteHub) && remoteHub->hasNodeID(deviceID)) {
		hubID = remoteHub->getID();
		return true;
	}

	std::vector<std::string> liveHubs;
	orbmanager->getAllLiveObjectContexts(stiContext, hubObjectName, liveHubs);



	//If found, rotate vector so inputHubID is first.
	auto pivot = std::find_if(liveHubs.begin(), liveHubs.end(), 
		[&inputHubID](const std::string& id) -> bool {
			return id == inputHubID;
		});

	if (pivot != liveHubs.end()) {
		std::rotate(liveHubs.begin(), pivot, pivot + 1);
	}
	else {
		std::cout << "Hub not found! " << inputHubID << std::endl;
	}

	for (auto& hubContext : liveHubs) {
		std::cout << "Live Hub: " << hubContext << std::endl;
		
		if (getRemoteHub(hubContext, remoteHub) && remoteHub->hasNodeID(deviceID)) {
			hubID = remoteHub->getID();
			found = true;
			break;
		}
	}


	return found;
}

void NetworkDeviceHub::getDeviceIDs(std::set<DeviceID>& ids) const
{
	localHub->getNodeIDs(ids);
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
	STI::TNetwork::TDeviceHub_var tDeviceHubLocal;

	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(deviceHubWrapper, tDeviceHubLocal)) {
		std::cerr
			<< "Error: Unable to get TDeviceHub reference from servant." << std::endl;
		return false;
	}

	// (1) Bind primary reference to this Hub under root context
	if (persistence.bindToRootContext) {
		success = orbmanager->bindObjectReference(thisHubContext, tDeviceHubLocal);
	}
	else {
		success = true;
	}


	// (2) Bind reference to this Hub under all target Hub contexts (for reconnect when target Hub restarts)
	for (auto& targetHub : targetHubs) {

		std::string targetHubPath = makeHubContextPath(stiContext, targetHub);

		//Add reference to this Hub under the target hub context (for rebind if target hub restarts)
		if (persistence.bindToTargetContexts) {
			success &= orbmanager->bindObjectReference(
				makeHubContext(targetHubPath, localHub->getID()),
				tDeviceHubLocal);
		}
		else {
			success &= true;
		}
	}

	return success;
}

void NetworkDeviceHub::shutdown()
{
	// std::set<DeviceID> ids;
	// getDeviceIDs(ids);

	// for(auto id : ids) {
	// 	deviceHubWrapper->removeNode(id);
	// }
	if (localHub != 0) {
		localHub->clear();
	}

	if (orbmanager != 0 && orbmanager->running()) {
		orbmanager->shutdown();		// fixes slow shutdown in windows
	}
}

void NetworkDeviceHub::run(bool block)
{
	if (orbmanager != 0 && orbmanager->running()) {
		return;
	}
	else if (orbmanager != 0 && orbmanager->initialized()) {
		orbmanager = ORBManager::getInstance();
	}
	else {
		return;
	}

	if (orbmanager == 0) {
		return;
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

	if (block && orbmanager->running()) {
		orbmanager->block();
	}
}

void NetworkDeviceHub::connectToTargetHubs()
{
	//Reconnect to target Hubs if they are live
	for (auto& targetHubID : targetHubs) {

		if (!localHub->containsHub(targetHubID)) {
			connectRemoteHub( makeHubContext(stiContext, targetHubID) );
		}
	}
}

std::string NetworkDeviceHub::makeHubContextPath(const std::string& baseContext, const HubID& hubID)
{
	std::stringstream hubContext;
	
	hubContext << baseContext << "/" << STI::Utils::replaceChars(hubID.getID(), ".", "_");

	return hubContext.str();
}


std::string NetworkDeviceHub::makeHubContext(const std::string& baseContext, const HubID& hubID)
{
	std::stringstream hubContext;

	hubContext << makeHubContextPath(baseContext, hubID) << "/" << hubObjectName;

	return hubContext.str();
}



bool NetworkDeviceHub::connectRemoteHub(const std::string& remoteHubContext)
{
	bool success = false;

	std::shared_ptr<RemoteDeviceHub> remoteHub;
	if (getRemoteHub(remoteHubContext, remoteHub)) {
		success = LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
	}

	return success;
}

bool NetworkDeviceHub::getRemoteHub(const std::string& remoteHubContext, std::shared_ptr<RemoteDeviceHub>& remoteHub)
{
	bool success = false;

	STI::TNetwork::TDeviceHub_ptr tDeviceHubRemote; // = STI::TNetwork::TDeviceHub::_nil();
	CORBA::Object_ptr obj;

	//Attempt to find and connect to live hub with remoteHubContext in NameService
	if (orbmanager->getObjectReference(remoteHubContext, obj)) {

		tDeviceHubRemote = STI::TNetwork::TDeviceHub::_narrow(obj);

		if (!CORBA::is_nil(tDeviceHubRemote)) {
			success = true;
			remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);
		}
	}

	return success && remoteHub != 0;
}

void NetworkDeviceHub::walk(LocalDeviceHub::HubNodeWalker& root) const
{
	if (localHub != 0) {
		localHub->walk(root);
	}
}

