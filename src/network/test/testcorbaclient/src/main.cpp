
#include "NetworkDeviceHub.h"

#include "LocalCollection.h"
#include "DeviceCollection.h"
#include "LocalDeviceHub.h"

#include <iostream>
#include <memory>
#include <string>

//#include "ORBManager.h"

//#include <signal.h>

using std::cout;
using std::endl;

class TempPolicy : public STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>::LocalCollectionPolicy
{
	bool include(const STI::Device::DeviceID& key) const { return true; }
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
};

class LocalDevice : public STI::Device::Device
{
public:
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : id(name, address, module, targetServer)
	{
		std::shared_ptr<TempPolicy> policy = std::make_shared<TempPolicy>();;
		localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>(policy);
	}
	~LocalDevice()
	{
		cout << "Destructor: " << id.getName() << endl;
	}
	STI::Device::DeviceID id;

	bool refresh() { return true; }

	void write(unsigned input)
	{
		cout << "writting: " << input << endl;
	}
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
	{
		collection = localCollection;
	}
	std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};

//void signal_callback_handler(int signum) {
//	cout << "Caught signal " << signum << endl;
//	// Terminate program
//	exit(signum);
//}

int main(int argc, char **argv)
{
//	auto dev0 = std::make_shared<LocalDevice>("dev0", "localhost", 0, "root");

	auto dev1 = std::make_shared<LocalDevice>("dev1", "localhost", 0, "localhost/0/dev0");
	auto dev2 = std::make_shared<LocalDevice>("dev2", "localhost", 0, "localhost/0/dev0");
//	auto dev3 = std::make_shared<LocalDevice>("dev3", "localhost", 0, "srv1");
//	auto dev4 = std::make_shared<LocalDevice>("dev4", "localhost", 0, "srv1");

	STI::Network::NetworkDeviceHub hub("192.168.1.6:2809");
	STI::Network::NetworkDeviceHub hub3("192.168.1.6:2809");


	//hub.setTargetHubs({"192.168.1.3/0/MAGIS","192.168.1.7/1/TestHub"});

	hub.addNode(dev1->id, dev1);
	//hub.addNode(dev2->id, dev2);
	
	hub3.addNode(dev2->id, dev2);

	hub3.run(false);
	hub.run(true);

	
	//std::shared_ptr<STI::Device::DeviceCollection> collection;
	//dev0->getCollection(collection);

	//std::shared_ptr<STI::Device::Device> devRef;
	//
	//std::set<STI::Device::DeviceID> ids;
	//collection->getIDs(ids);
	//collection->get(, devRef)

	//signal(SIGINT, signal_callback_handler);

	//hub.block();

//	hub.orbmanager->shutdown();

	//auto hub2 = std::make_shared<STI::Network::LocalDeviceHub>("Hub2");
	//
	//hub2->addNode(dev3->id, dev3);
	//hub2->addNode(dev4->id, dev4);

	//hub.connect(hub2);

	//hub2->clear();





	//hub.connect(otherHub);
	//hub.connect(device);
	//hub.run();		//blocking


//	CORBA::Object_var obj;
//	obj = orbManager->getObjectReference("STI/Network/TServer.Object");
//	::STI::Network::TServer_var tServerRef;
//	tServerRef = STI::Network::TServer::_narrow(obj);

//	orbManager->registerServant(remoteDeviceServant.get(), contextName + deviceBootstrapObjectName);


//	auto hub1 = std::make_shared<STI::Network::LocalDeviceHub>("Hub1");

//	STI::Network::NetworkDeviceHubWrapper hub(hub1);

	return 0;
}