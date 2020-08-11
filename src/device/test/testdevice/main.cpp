
#include "Device.h"
#include "LocalCollection.h"

#include <iostream>
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
	
	bool refresh()
	{
		return true;
	}

	void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	{
		collection = localCollection;
	}
	std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};


int main(int argc, char **argv)
{
	LocalDevice dev1("test", "localhost", 0, "localhost/0/server");

	dev1.write(22);


	return 0;
}