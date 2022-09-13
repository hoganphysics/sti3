
#include <sti/NetworkDeviceHub.h>

#include <sti/utils/LocalCollection.h>
#include <sti/device/DeviceCollection.h>
#include <sti/LocalDeviceHub.h>
#include <sti/LocalDevice.h>
#include "DeviceEvent.h"
#include "DeviceEventDispatcher.h"
#include "DeviceEventReceiver.h"

#include <iostream>
#include <memory>
#include <string>

using std::cout;
using std::endl;


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

//class TempPolicy : public STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>::LocalCollectionPolicy
//{
//	bool include(const STI::Device::DeviceID& key) const { return true; }
//	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
//};

class TestDevice : public STI::Device::LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) /*: id(name, address, module, targetServer)*/
		: STI::Device::LocalDevice(name, address, module, targetServer)
	{
		//std::shared_ptr<TempPolicy> policy = std::make_shared<TempPolicy>();;
		//localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>(policy);
	}
	~TestDevice()
	{
		cout << "Destructor: " << id.getName() << endl;
	}
	//STI::Device::DeviceID id;

	//bool refresh() { return true; }

	void fireRefreshEvent()
	{
		std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
		getEventDispatcher(dispatcher);

		auto evt = std::make_shared<STI::Device::RefreshDeviceEvent>(id);
		dispatcher->addEvent(evt);
	}

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
	auto dev0 = std::make_shared<TestDevice>("dev0", "localhost", 0, "root");

	auto dev1 = std::make_shared<TestDevice>("dev1", "localhost", 0, "localhost/0/dev0");
	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "localhost/0/dev0");
	auto dev3 = std::make_shared<TestDevice>("dev3", "localhost", 0, "localhost/0/dev0");
	auto dev4 = std::make_shared<TestDevice>("dev4", "localhost", 0, "localhost/0/dev0");

	auto hub  = std::make_shared<STI::Network::LocalDeviceHub>("hub1", "localhost", 0);
	auto hub2 = std::make_shared<STI::Network::LocalDeviceHub>("hub2", "localhost", 0);

//	STI::Network::NetworkDeviceHub hub3("192.168.1.6:2809");


	//hub.setTargetHubs({"192.168.1.3/0/MAGIS","192.168.1.7/1/TestHub"});

	hub->addNode(dev0->id, dev0);
	hub->addNode(dev1->id, dev1);
	
	
	hub2->addNode(dev2->id, dev2);
	hub2->addNode(dev3->id, dev3);
	hub2->addNode(dev4->id, dev4);

	STI::Network::DeviceHub::connect(hub, hub2);

	STI::Network::LocalDeviceHub::HubNodeWalker deviceGraph;
	
	hub->walk(deviceGraph);

	int tmp;
	std::cin >> tmp;
	

	return 0;
}