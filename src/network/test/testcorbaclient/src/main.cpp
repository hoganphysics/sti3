
#include "NetworkDeviceHub.h"

#include "LocalCollection.h"
#include "DeviceCollection.h"
#include "LocalDeviceHub.h"
#include "LocalDevice.h"
#include "DeviceEventListener.h"
#include "DeviceEventReceiver.h"

#include <iostream>
#include <memory>
#include <string>

//#include "ORBManager.h"

//#include <signal.h>

using std::cout;
using std::endl;

//class TempPolicy : public STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>::LocalCollectionPolicy
//{
//	bool include(const STI::Device::DeviceID& key) const { return true; }
//	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
//};


class TestListener : public STI::Device::DeviceEventListener<STI::Device::RefreshDeviceEvent>
{
public:
	TestListener(const std::string& label) : label(label) {}

	void handleEvent(const std::shared_ptr<STI::Device::RefreshDeviceEvent>& evt)
	{
		std::unique_lock < std::mutex > writeLock(TestListener::coutMutex);
		cout << "Refresh " << label << ". Source: " << evt->sourceID().getName() << endl;
	}

	std::string label;
	static std::mutex coutMutex;
};

std::mutex TestListener::coutMutex{};


class TestDevice : public STI::Device::LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : LocalDevice(name, address, module, targetServer)
	{
		//std::shared_ptr<TempPolicy> policy = std::make_shared<TempPolicy>();;
		//localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>(policy);
		
		std::string lname = "listener:" + name;
		listener = std::make_shared<TestListener>(lname);

		std::shared_ptr<STI::Device::DeviceEventReceiver> receiver;
		getEventReceiver(receiver);

		std::shared_ptr<STI::Device::DeviceEventListener<STI::Device::RefreshDeviceEvent>> listenerX = listener;

		STI::Device::DeviceEventListenerID listenerID;
		listenerID.name = "listener_1";
		listenerID.type = STI::Device::DeviceEventType::Refresh;

		receiver->addListener(STI::Device::DeviceID("dev0", "localhost", 0, ""), listenerID, listenerX);

	}
	~TestDevice()
	{
		cout << "Destructor: " << id.getName() << endl;
	}
	//STI::Device::DeviceID id;

	//bool refresh() { return true; }

	std::shared_ptr<TestListener> listener;

	void write(unsigned input)
	{
		cout << "writting: " << input << endl;
	}
	//void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
	//{
	//	collection = localCollection;
	//}
	//std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};

//void signal_callback_handler(int signum) {
//	cout << "Caught signal " << signum << endl;
//	// Terminate program
//	exit(signum);
//}

int main(int argc, char **argv)
{
//	auto dev0 = std::make_shared<LocalDevice>("dev0", "localhost", 0, "root");

	auto dev1 = std::make_shared<TestDevice>("dev1", "localhost", 0, "localhost/0/dev0");
	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/dev0");
//	auto dev3 = std::make_shared<LocalDevice>("dev3", "localhost", 0, "srv1");
//	auto dev4 = std::make_shared<LocalDevice>("dev4", "localhost", 0, "srv1");

	STI::Network::NetworkDeviceHub hub("192.168.1.6:2809");
	//STI::Network::NetworkDeviceHub hub3("192.168.1.6:2809");


	//hub.setTargetHubs({"192.168.1.3/0/MAGIS","192.168.1.7/1/TestHub"});

	hub.addNode(dev1->id, dev1);
	hub.addNode(dev2->id, dev2);
	
	//hub3.addNode(dev2->id, dev2);

	std::shared_ptr<STI::Device::DeviceEventReceiver> receiver;


	//hub3.run(false);
	hub.run(true);

/*

	int tmp;
	std::cin >> tmp;

	dev1->getEventReceiver(receiver);
	receiver->addDeviceEventHandler(STI::Device::DeviceID("dev0", "localhost", 0, ""));

	std::cin >> tmp;

	dev1->getEventReceiver(receiver);
	receiver->addDeviceEventHandler(STI::Device::DeviceID("dev0", "localhost", 0, ""));

	std::cin >> tmp;*/


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