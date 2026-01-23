#include <sti/NetworkDeviceHub.h>
#include <sti/LocalDeviceHub.h>
#include <sti/device/DeviceID.h>
#include <sti/network/DeviceHub.h>

#include <sti/utils/TaskScheduler.h>
#include <sti/utils/IntervalTask.h>

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

	persistence.bindToRootContext = true;
	persistence.bindToTargetContexts = true;

	// STI::Utils::Configuration omniConfig(config.getParameters("omniORB"));
	STI::Utils::Configuration omniConfig;
	omniConfig.set("InitRef", "NameService=corbaname::" + _nameServiceAddress);
	omniConfig.set("omniORB", "clientConnectTimeOutPeriod", "500");

	//Add parameters from config file (overwrites any duplicate entries)
	omniConfig.append(STI::Utils::Configuration().append( config.getParameters("omniORB") ));

	orbmanager = STI::Network::ORBManager::getInstance(omniConfig, "");

	localHub = std::make_shared<LocalDeviceHub>(hubID);
	
	if (orbmanager->initialized()) {
		deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
	}

	refreshHubContext();

	// Refresh local hub periodically to remove dead references
	int refreshTime = 5;	//seconds
	auto refreshTask = std::make_shared<STI::Utils::IntervalTask>("refresh", refreshTime,
		[this]() {
			localHub->refresh();

			if (localHub->numberOfNodes() == 0) {
				//Terminate hub if no nodes are left
				unblock();		// unblock run() if it is blocking
			}
		});
	
	refreshScheduler = std::make_shared<STI::Utils::TaskScheduler>();
	refreshScheduler->start();
	refreshScheduler->addTask(refreshTask);
}

NetworkDeviceHub::~NetworkDeviceHub()
{
	// if (localHub != 0) {
	// 	localHub->clear();
	// }
	
	shutdown();

	if (refreshScheduler != 0) {
		refreshScheduler->stop();
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
	
	// return "";

	STI::Utils::Configuration omniConfig;
	omniConfig.set("InitRef", "NameService=corbaname::" + nameServiceAddress);
	omniConfig.set("omniORB", "clientConnectTimeOutPeriod", "500");

	auto orbmanager = STI::Network::ORBManager::getInstance(omniConfig, "");

	if (orbmanager != 0) {
		return orbmanager->printNameTree(baseContext);
	}
	return "";
}

STI::Network::HubID NetworkDeviceHub::getID() const
{
	if (localHub != 0) {
		return localHub->getID();
	}
	return STI::Network::HubID();
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
	if (node == 0) return false;
	
	HubID serverHubID;
	std::string nodeTargetServerID = node->getID().getTargetServerID();		//initial guess

	if (useAutoTargetHubIDs && nodeTargetServerID.compare("root") != 0) {

		HubID::stringToHubID(nodeTargetServerID, serverHubID);
		
		// Attempt to find the hub that hosts the target server ID; only work if it's already connected
		if (!findHub(nodeTargetServerID, serverHubID)) {
			std::cerr << "Warning: Unable to find target hub for device " 
				<< node->getID().getID() << " with target server ID " 
				<< nodeTargetServerID << std::endl;
			std::cerr << "Adding device to default hub context: " << serverHubID.getID() << std::endl;
		}
	}

	return addDevice(node, serverHubID);

	if (node != 0) {
		return addNode(node->getID(), node);
	}
	return false;
}

bool NetworkDeviceHub::addDevice(const typename std::shared_ptr<STI::Device::Device>& node, const std::string& serverHubID)
{
	HubID hubID;
	HubID::stringToHubID(serverHubID, hubID);

	return addDevice(node, hubID);
}

bool NetworkDeviceHub::addDevice(const typename std::shared_ptr<STI::Device::Device>& node, const HubID& serverHubID)
{
	bool success = false;

	if (node != 0) {
		success = addNode(node->getID(), node);
	}

	if (success && serverHubID.isValid()) {
		addTargetHub(serverHubID);		//The hub that hosts this device's server
	}

	return success;
}

bool NetworkDeviceHub::addNode(const DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (_usingDefaultHubID && localHub != 0 && localHub->numberOfNodes() == 0) {
		//First attached node, and the NetworkDeviceHub is configured with a default HubID
		//By default, the HubID is set to derive from the DeviceID of the first added node,
		//with a prefix prepended to the device name as follows:
		// DeviceID = localhost/0/dev1
		// HubID = localhost/0/Hub::dev1
		// The prefix is automatically added by the HubID class.

		HubID newHubID(id.getName(), id.getAddress(), id.getModule());
		localHub->setID(newHubID);
		refreshHubContext();
	}

	bool success = false;

	if (localHub != 0 && deviceHubWrapper != 0) {
		success = deviceHubWrapper->addNode(id, node);
	}
	
	return success;
}


bool NetworkDeviceHub::findHub(const STI::Device::DeviceID& deviceID, HubID& hubID)
{
	bool found = false;
	std::shared_ptr<RemoteDeviceHub> remoteHub;
	std::stringstream errors;

	//try hint HubID first, if it is live
	auto inputHubID = makeHubContext(stiContext, hubID);

	if (getRemoteHub(inputHubID, remoteHub, errors) && remoteHub->hasNodeID(deviceID)) {
		hubID = remoteHub->getID();
		return true;
	}

	if (orbmanager == nullptr) {
		return false;
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

	for (auto& hubContext : liveHubs) {	
		if (getRemoteHub(hubContext, remoteHub, errors) && remoteHub->hasNodeID(deviceID)) {
			hubID = remoteHub->getID();
			found = true;
			break;
		}
	}

	return found;
}

void NetworkDeviceHub::getDeviceIDs(std::set<DeviceID>& ids) const
{
	if (localHub == 0) return;

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

	if (orbmanager == nullptr) {
		return false;
	}

	// (1) Bind primary reference to this Hub under root context
	if (persistence.bindToRootContext) {
		success = orbmanager->bindObjectReference(thisHubContext, tDeviceHubLocal.in());
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
				tDeviceHubLocal.in());
		}
		else {
			success &= true;
		}
	}

	return success;
}

void NetworkDeviceHub::shutdown()
{
	disconnect();

	// Keep the ORB alive so it can be reused across hub instances.
	unblock();
}

void NetworkDeviceHub::disconnect()
{
	if (localHub != 0) {
		localHub->clear();
		localHub->disconnect();
	}
}

void NetworkDeviceHub::unblock()
{
	if (orbmanager != 0 && orbmanager->blocking()) {
		orbmanager->unblock();
	}
}

void NetworkDeviceHub::run(bool block)
{
	if (orbmanager == 0) {
		orbmanager = ORBManager::getInstance();
	}

	if (orbmanager == 0) return;

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
	if (!orbmanager->running()) {
		orbmanager->run();	//doesn't block
	}

	if (block && orbmanager->running() && !orbmanager->blocking()) {
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
	if (getRemoteHub(remoteHubContext, remoteHub, std::cerr)) {
		success = LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
	}

	return success;
}

bool NetworkDeviceHub::getRemoteHub(const std::string& remoteHubContext, std::shared_ptr<RemoteDeviceHub>& remoteHub, std::ostream& errorBuf)
{
	bool success = false;

	// STI::TNetwork::TDeviceHub_var tDeviceHubRemote; // = STI::TNetwork::TDeviceHub::_nil();
	CORBA::Object_var obj;

	//Attempt to find and connect to live hub with remoteHubContext in NameService
	if (orbmanager->getObjectReference(remoteHubContext, obj, errorBuf)) {

		STI::TNetwork::TDeviceHub_var tDeviceHubRemote = STI::TNetwork::TDeviceHub::_narrow(obj);

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

