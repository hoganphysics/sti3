


//#include <sti/network/Hub.h>
//#include <sti/device/Device.h>
#include <sti/utils/LocalCollection.h>
//#include <sti/network/LocalHub.h>
#include <sti/LocalDeviceHub.h>

#include <sti/LocalDevice.h>

#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessageDispatcher.h>


#include <iostream>
#include <memory>
#include <string>
#include <mutex>

using std::cout;
using std::endl;
using STI::Device::LocalDevice;

class TestListener : public STI::Device::DeviceMessageListener<STI::Device::RefreshDeviceMessage>
{
public:
	TestListener(const std::string& label) : label(label) {}

	void handleMessage(const std::shared_ptr<STI::Device::RefreshDeviceMessage>& evt)
	{
		std::unique_lock < std::mutex > writeLock(TestListener::coutMutex);
		cout << "Refresh " << label << ". Source: " << evt->sourceID().getName() << endl;
	}

	std::string label;
	static std::mutex coutMutex;
};

std::mutex TestListener::coutMutex{};


class TestListener2 : public STI::Device::DeviceMessageListener<STI::Device::ChannelUpdateDeviceMessage>
{
public:
	TestListener2(const std::string& label) : label(label) {}

	void handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateDeviceMessage>& evt)
	{
	}
	std::string label;
};


class TestDevice : public LocalDevice
{
public:
	TestDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : LocalDevice(name, address, module, targetServer)
	{
		std::string lname = "listener:" + name;
		listener = std::make_shared<TestListener>(lname);
		listener2 = std::make_shared<TestListener2>(lname);

		std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
		getMessageReceiver(receiver);

		std::shared_ptr<STI::Device::DeviceMessageListener<STI::Device::RefreshDeviceMessage>> listenerX = listener;
		std::shared_ptr<STI::Device::DeviceMessageListener<STI::Device::ChannelUpdateDeviceMessage>> listener2X = listener2;

		STI::Device::DeviceMessageListenerID listenerID;
		listenerID.name = "listener_1";
		listenerID.type = STI::Device::DeviceMessageType::Refresh;

		receiver->addListener(STI::Device::DeviceID("dev2", "localhost", 0, ""), listenerID, listenerX);
		
		listenerID.type = STI::Device::DeviceMessageType::ChannelUpdate;
		receiver->addListener(STI::Device::DeviceID("dev2", "localhost", 0, ""), listenerID, listener2X);
		//receiver->removeListener(STI::Device::DeviceID("dev2", "localhost", 0, ""), listenerID);

		//receiver->addListener(STI::Device::DeviceID("dev1", "localhost", 0, ""), "listener_2", listenerX);

	}
	~TestDevice()
	{
		cout << "Destroying " << getID().getName() << endl;
	}

	void fireRefreshEvent()
	{
		std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
		getMessageDispatcher(dispatcher);

		auto mess = std::make_shared<STI::Device::RefreshDeviceMessage>(getID());
		dispatcher->addMessage(mess);
	}
	
	std::shared_ptr<TestListener> listener;

	std::shared_ptr<TestListener2> listener2;

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

	hub1->addNode(dev1->getID(), dev1);
	hub1->addNode(dev2->getID(), dev2);

	hub2->addNode(dev3->getID(), dev3);

	STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>::connect(hub1, hub2);


	hub2->addNode(dev4->getID(), dev4);

	int x;
	std::cin >> x;


	dev2->fireRefreshEvent();

	hub1->refresh();

	hub2->removeNode(dev3->getID());

	std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>> testCollection;
	dev1->getCollection(testCollection);

	std::shared_ptr<STI::Device::Device> p1;
	std::shared_ptr<STI::Network::Node<STI::Device::DeviceID, STI::Device::Device>> p2;
	testCollection->get(dev2->getID(), p1);
	//	testCollection->get(dev2->id, p2);

	p2 = p1;

	//p1->write(1);
	//p2->get().write(2);

	//(*p2)->write(3);
	//p2.get()->get().write(4);


	cout.flush();


	hub1->clear();
	hub2->clear();


	return 0;
}

