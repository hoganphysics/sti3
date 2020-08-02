
#include "Hub.h"
#include "Device.h"
#include "LocalCollection.h"
#include "Hub.h"

#include <iostream>
#include <memory>
#include <string>

using std::cout;
using std::endl;

class LocalDevice : public STI::Device::Device
{
public:
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : id(name, address, module, targetServer)
	{
		localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>();
	}
	~LocalDevice()
	{
		cout << "Destructor: " << id.getName() << endl;
	}
	STI::Device::DeviceID id;
	void write(unsigned input)
	{
		cout << "writting: " << input << endl;
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	{
		collection = localCollection;
	}
	std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};

class LocalHub : public STI::Utils::Hub<STI::Device::DeviceID, STI::Device::Device>
{
public:
	LocalHub(const std::string& name) : id(name, "", 0, "")
	{
		localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>();
	}

	const STI::Device::DeviceID& getID() { return id; }
	STI::Device::DeviceID id;
	//void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	//{
	//	collection = localCollection;
	//}
	std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};

int main(int argc, char **argv)
{
	auto dev1 = std::make_shared<LocalDevice>("dev1", "localhost", 0, "srv1");
	auto dev2 = std::make_shared<LocalDevice>("dev2", "localhost", 0, "srv1");
	auto dev3 = std::make_shared<LocalDevice>("dev3", "localhost", 0, "srv1");

//	STI::Utils::Distributer<STI::Device::DeviceID, STI::Device::Device> dist;
//	dist.add(dev1->id, dev1);
//	dist.add(dev2->id, dev2);

	auto hub1 = std::make_shared<LocalHub>("Hub1");
	auto hub2 = std::make_shared<LocalHub>("Hub2");
	//LocalHub hub1("Hub1");
	//LocalHub hub2("Hub2");

	hub1->addNode(dev1->id, dev1);
	hub1->addNode(dev2->id, dev2);

	hub2->addNode(dev3->id, dev3);

	hub1->addHub(hub2->id, hub2);
//	hub2->addHub(hub1->id, hub1);		//infinite recursion!

	std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>> testCollection;
	dev1->getCollection(testCollection);

	std::shared_ptr<STI::Device::Device> p1;
	std::shared_ptr<STI::Utils::Node<STI::Device::DeviceID, STI::Device::Device>> p2;
	testCollection->get(dev2->id, p1);
//	testCollection->get(dev2->id, p2);

	p2 = p1;

	p1->write(1);
	p2->get().write(2);

	(*p2)->write(3);
	p2.get()->get().write(4);

	return 0;
}

//
//template<class ID, class T> class Hub;
//
//template<class ID, class T>
//class Hub
//{
//	typedef std::shared_ptr<T> T_ptr;
//
//	bool addNode(const ID& id, const T_ptr& node);
//	bool remove(const ID& id);
//	bool addHub(const ID& id, const typename std::shared_ptr<Hub<ID, T>>& hub);
//
//	void refresh();
//	void removeAll();
//};
//
//
