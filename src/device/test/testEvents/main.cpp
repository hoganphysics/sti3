


//#include "Hub.h"
//#include "Device.h"
#include "LocalCollection.h"
//#include "LocalHub.h"
#include "LocalDeviceHub.h"

#include "LocalDevice.h"

#include "DeviceEventListener.h"
#include "DeviceEvent.h"
#include "DeviceEventReceiver.h"
#include "DeviceEventDispatcher.h"


#include <iostream>
#include <memory>
#include <string>

using std::cout;
using std::endl;
using STI::Device::LocalDevice;

class TestListener : public STI::Device::DeviceEventListener<STI::Device::RefreshDeviceEvent>
{
public:
	TestListener(const std::string& label) : label(label) {}

	void handleEvent(const STI::Device::RefreshDeviceEvent& evt)
	{
		
		cout << "Refresh " << label << ". Source: " << evt.sourceID().getID() << endl;
	}

	std::string label;
};

class TestDevice : public LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : LocalDevice(name, address, module, targetServer)
	{

		listener = std::make_shared<TestListener>("testListener");

		std::shared_ptr<STI::Device::DeviceEventReceiver> receiver;
		getEventReceiver(receiver);

		std::shared_ptr<STI::Device::DeviceEventListener<STI::Device::RefreshDeviceEvent>> listener2 = listener;
		receiver->addListener(STI::Device::DeviceID("dev1", "localhost", 0, ""), "listener_1", listener2);
	}

	void fireRefreshEvent()
	{
		std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
		getEventDispatcher(dispatcher);

		dispatcher->addEvent( STI::Device::RefreshDeviceEvent(id) );
	}
	
	std::shared_ptr<TestListener> listener;

};

int main(int argc, char **argv)
{
	auto dev1 = std::make_shared<TestDevice>("dev1", "localhost", 0, "srv1");
	auto dev2 = std::make_shared<TestDevice>("dev2", "localhost", 0, "srv1");
	auto dev3 = std::make_shared<TestDevice>("dev3", "localhost", 0, "srv1");
	auto dev4 = std::make_shared<TestDevice>("dev4", "localhost", 0, "srv1");

	//	STI::Utils::Distributer<STI::Device::DeviceID, STI::Device::Device> dist;
	//	dist.add(dev1->id, dev1);
	//	dist.add(dev2->id, dev2);

	auto hub1 = std::make_shared<STI::Network::LocalDeviceHub>("Hub1");
	auto hub2 = std::make_shared<STI::Network::LocalDeviceHub>("Hub2");
	//LocalHub hub1("Hub1");
	//LocalHub hub2("Hub2");

	hub1->addNode(dev1->id, dev1);
	hub1->addNode(dev2->id, dev2);

	hub2->addNode(dev3->id, dev3);

	STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>::connect(hub1, hub2);

	dev1->fireRefreshEvent();

	hub2->addNode(dev4->id, dev4);

	hub1->refresh();

	hub2->removeNode(dev3->id);

	std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>> testCollection;
	dev1->getCollection(testCollection);

	std::shared_ptr<STI::Device::Device> p1;
	std::shared_ptr<STI::Network::Node<STI::Device::DeviceID, STI::Device::Device>> p2;
	testCollection->get(dev2->id, p1);
	//	testCollection->get(dev2->id, p2);

	p2 = p1;

	p1->write(1);
	p2->get().write(2);

	(*p2)->write(3);
	p2.get()->get().write(4);


	hub1->clear();
	hub2->clear();


	return 0;
}

