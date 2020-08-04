
#include "NetworkDeviceHub.h"

#include "LocalCollection.h"
#include "DeviceCollection.h"
#include "LocalDeviceHub.h"

#include <iostream>
#include <memory>
#include <string>

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


int main(int argc, char **argv)
{
	auto dev1 = std::make_shared<LocalDevice>("dev1", "localhost", 0, "srv1");
	auto dev2 = std::make_shared<LocalDevice>("dev2", "localhost", 0, "srv1");
	auto dev3 = std::make_shared<LocalDevice>("dev3", "localhost", 0, "srv1");
	auto dev4 = std::make_shared<LocalDevice>("dev4", "localhost", 0, "srv1");

	STI::Network::NetworkDeviceHub hub("net hub");

	hub.addNode(dev1->id, dev1);
	hub.addNode(dev2->id, dev2);
	
	auto hub2 = std::make_shared<STI::Network::LocalDeviceHub>("Hub2");
	
	hub2->addNode(dev3->id, dev3);
	hub2->addNode(dev4->id, dev4);

	hub.connect(hub2);

	hub2->clear();

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