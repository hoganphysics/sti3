
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

std::string NetworkDeviceHub::nextHubName()
{
	hubNumber++;
	return (std::string("NetworkDeviceHub_") + STI::Utils::valueToString(hubNumber));
}
unsigned NetworkDeviceHub::hubNumber = 0;


NetworkDeviceHub::NetworkDeviceHub(const std::string& nameServiceAddress)
	: NetworkDeviceHub(NetworkDeviceHub::nextHubName(), "localhost", 0, nameServiceAddress)
{
	//Default HubID constuctor
//	_targetServerContext = "root";	//default;
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
	std::cerr << "Destroying NetworkDeviceHub" << std::endl;

	//Shutdown the ORB first, before deleting any servants stored by the NetworkDeviceHub.
	//Otherwise, the ORB will crash on shutdown, since it may have some references to 
	//the deleted servants.
	if (orbmanager != 0) {
//		orbmanager->shutdown();
	}
}


void NetworkDeviceHub::setTargetHubs(const std::vector<std::string>& hubIDs)
{ 
	for (auto& id : hubIDs) {
		targetHubs.insert(id);
	}
	//targetHubs = hubIDs;
}


//
//bool NetworkDeviceHub::setTargetServer(const std::string& serverContext)	//setTargetHub(context)
//{
//	_targetServerContext = serverContext;
//	return true;
///*
//	if (_targetServerContext.compare("root") == 0 
//		|| _targetServerContext.compare(serverContext) == 0) {
//		_targetServerContext = serverContext;
//		return true;
//	}
//	else {
//		std::cout
//			<< "Error: The Network Hub's manually set target server differs from the target server" << std::endl
//			<< "of the device(s) attached to the hub.  Each Network Hub must have a single target" << std::endl
//			<< "address and it must match the server target of all connected devices." << std::endl;
//	}
//	return false;*/
//}

void NetworkDeviceHub::autoReconnectRemoteHubs(bool enabled)
{ 
	_autoReconnectRemoteHubs = enabled;
}

bool NetworkDeviceHub::addNode(const DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (_usingDefaultHubID && localHub != 0 && localHub->numberOfNodes() == 0) {
		//First attached node, and the NetworkDeviceHub is configured with a default HubID
		//By default, the HubID is set to match the DeviceID of the first added node.
		HubID newHubID(id.getName(), id.getAddress(), id.getModule());
		localHub->setID(newHubID);
	}

	//if (localHub == 0) {
	//	//Hub not initialized yet (this must be the first attached node)
	//	
	//	//Use DeviceID to setup defaul HubID
	//	localHub = std::make_shared<LocalDeviceHub>(id.getName(), id.getAddress(), id.getModule());
	//	deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
	//}

	//if (_targetServerContext.compare("root") == 0) {
	//	//default target hub is th 
	//	_targetHubContext = id.getTargetServerName();		//Network Hub inherits the target server of the attached node
	//}
	//else if (_targetServerContext.compare(id.getTargetServerName()) != 0){
	//	std::cout
	//		<< "Error: Attemped to add a Device Node with target server address " << std:: endl
	//		<< "'" << id.getTargetServerName() << "'" << std::endl
	//		<< "To a Network Device Hub with at a target server address of "
	//		<< "'" << _targetServerContext << "'" << std::endl
	//		<< "Each Network Device Hub must have a single target server address and it must match the target" << std::endl
	//		<< "server address of all connected devices." << std::endl;
	//	return false;
	//}

	bool success = false;

	if (localHub != 0) {
		success = localHub->addNode(id, node);
	}

	if (success && _autoConnect) {
		//add to list of hubs this hub will attempt to connect to
		targetHubs.insert(id.getTargetServerID());
	}

	return success;
}

bool NetworkDeviceHub::connect(const std::shared_ptr<LocalDeviceHub>& hub)
{
	//could just wrap localHub with a NetworkDeviceHubWrapper here so the local hub
	//is transparently on the network.  Would still act local.
	//Problem: hubs connected to other local hubs...
	//**Probably better if we have a walker visitor that can gather Network connection info.
	return LocalDeviceHub::connect(localHub, hub);
}

//void NetworkDeviceHub::block()
//{ 
//	orbmanager->block();
//}


void NetworkDeviceHub::run(bool block)
{
	if (orbmanager != 0 && orbmanager->running()) {
		return;
	}
	else {
		//orbmanager = std::make_unique<ORBManager>(_nameServiceAddress, "");
		orbmanager = ORBManager::getInstance(_nameServiceAddress, "");
	}
	
//	orbmanager->run();

	//register this TDeviceHub with NameService
	std::stringstream hubContext;
	STI::TNetwork::TDeviceHub_ptr tDeviceHubLocal;	//var type instead?
	STI::TNetwork::TDeviceHub_ptr tDeviceHubRemote = STI::TNetwork::TDeviceHub::_nil();
	
	//orbmanager->poa->activate_object(&(deviceHubWrapper->deviceHubServant));

	hubContext << "STI" << "/" << localHub->getID().id() << "/" << "TDeviceHub.Object";
	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(deviceHubWrapper, tDeviceHubLocal)) {
		std::cerr
			<< "Error: Unable to get TDeviceHub reference from servant." << std::endl;
		return;
	}

	orbmanager->bindObjectReference(hubContext.str(), tDeviceHubLocal);

	hubContext.str("");
	hubContext.clear();
	hubContext << "STI" << "/" << localHub->getID().id() << "/";

	//Get TDeviceHub from all live hubs under Hub context and wrap them in RemoteHubs and reconnect
	std::vector<std::string> liveHubs;

	orbmanager->getAllLiveObjectContexts("STI", "TDeviceHub.Object", liveHubs);

	orbmanager->getAllLiveObjectContexts(hubContext.str(), "TDeviceHub.Object", liveHubs);
	std::shared_ptr<RemoteDeviceHub> remoteHub;
	
	//Auto reconnect to Hubs that have connected before
	for (auto& hubRefContextName : liveHubs) {
		if (orbmanager->getObjectReference(hubRefContextName, tDeviceHubRemote)) {
			remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);

			LocalDeviceHub::connect(remoteHub, deviceHubWrapper);

//			if (remoteHub != 0) {
				

//				localHub->addHub(remoteHub->getID(), remoteHub);					//add remote Hub to local
//				remoteHub->addHub(deviceHubWrapper->getID(), deviceHubWrapper);		//add local to remote
//			}
		}
	}


	//Add reference to this Hub under all target Hubs (for rebind if target Hub restarts)
	//Reconnect to target Hubs if they are live
	for (auto& targetHub : targetHubs) {
		hubContext.str("");
		hubContext.clear();

		hubContext << "STI" << "/" << STI::Utils::replaceChars(targetHub, ".", "_") << "/";

		//Reconnect to target hub if it is live
		if (orbmanager->getObjectReference(hubContext.str() + "TDeviceHub.Object", tDeviceHubRemote)) {
			remoteHub = std::make_shared<RemoteDeviceHub>(tDeviceHubRemote);

			LocalDeviceHub::connect(remoteHub, deviceHubWrapper);
		}

		//Add reference to this Hub under th target hub context (for rebind if target hub restarts)
		hubContext << localHub->getID().id() << "/" << "TDeviceHub.Object";

		orbmanager->bindObjectReference(hubContext.str(), tDeviceHubLocal);
	}

	orbmanager->getAllLiveObjectContexts("STI", "TDeviceHub.Object", liveHubs);

	//Get 

	//For all Hubs:
	//STI/<Hub ID>/TDeviceHub.Object
	// Hub ID must match target server ID of device hubs..

	//All devices connected to a hub must have the same target server id.  The hub inherits this.
	//
	//Hubs connect to hubs.

	//If not root:
	//STI/<Target Server ID>/<Hub ID>/TDeviceHub.Object
	//All devices go under: (<Hub ID> can find, and )
	//STI/<Target Server ID>/<Hub ID>/<Device ID>/TDeviceHub.Object

	

	//
	//
	//std::stringstream serverContext;

	//serverContext << "STI" << "/";

	////Register the DeviceHub
	//if (_targetServerContext.compare("root") != 0) {
	//	serverContext << _targetServerContext << "/";
	//}
	//
	////DON'T WANT TO DO THIS!

	////Register servants for all owned Devices
	//std::set<STI::Device::DeviceID> deviceIDs;
	//localHub->getNodeIDs(deviceIDs);

	//for (auto& id : deviceIDs) {

	//}

	//for (auto& id : deviceIDs) {
	//	STI::Device::DeviceID::generateContext(id);
	//}

	orbmanager->run();	//doesn't block

	if (block) {
		orbmanager->block();
	}
}

//
//void NetworkDeviceHub::getAllLiveRemoteHubs(const std::string& baseContext, std::vector<std::shared_ptr<DeviceHub>>& liveHubs)
//{
//
//	STI::TNetwork::TDeviceHub_ptr remoteHubRef;
//	CORBA::Object_var obj;
//	std::shared_ptr<RemoteDeviceHub> remoteHub;
//
//	std::vector<std::string> hubNames;
//	getAllLiveRemoteHubNames(baseContext, hubNames);
//	
//	for (auto& name : hubNames) {
//		obj = orbManager->getObjectReference(name);
//		remoteHubRef = STI::TNetwork::TDeviceHub::_narrow(obj);
//
//		if (!CORBA::is_nil(remoteHubRef)) {
//
//			remoteHub = std::make_shared<RemoteDeviceHub>(remoteHubRef);
//
//			if (remoteHub != 0) {
//				liveHubs.push_back(remoteHub);
//			}
//		}
//	}
//}
//
//void NetworkDeviceHub::getAllLiveRemoteHubNames(const std::string& baseContext, std::vector<std::string>& hubNames)
//{
//	//	orbmanager
//	COSBindingNode node;
//
//	for (node : nodes) {
//		if (node == hub) {
//			addName(node.name)
//		}
//		else {
//			getAllLiveRemoteHubNames(node, hubNames, currentName+node.name);
//		}
//	}
//}
//
//CORBA::Object_ptr ORBManager::getObjectReference(const std::string& objectStringName)
//{
//
//}
//
//STI::Network::TRemoteDevice_ptr remoteDeviceRef;
//CORBA::Object_var obj;
//
//obj = orbManager->getObjectReference(deviceContext.str());
//remoteDeviceRef = STI::Network::TRemoteDevice::_narrow(obj);

