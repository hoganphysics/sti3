
#include "ORBManager.h"
#include "TDeviceHub_i.h"
#include <sti/LocalDeviceHub.h>

#include "TDeviceCollection_i.h"

#include <memory>

using namespace STI::Network;

class TempCollector : public STI::Device::DeviceCollector
{
	void getCollection(std::shared_ptr<STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device>>& collection)
	{

	}
};

int main(int argc, char **argv)
{

	//auto orbMan = ORBManager::getInstance("192.168.1.6:2809", "");
	
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
	CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
	PortableServer::POAManager_var pman = poa->the_POAManager();
	pman->activate();

//	std::shared_ptr<DeviceHub> localHub = std::make_shared<LocalDeviceHub>("Hub1", "localhost", 0);

//	std::shared_ptr<STI::TNetwork::TDeviceHub_i> deviceHubServant 
//		= std::make_shared<STI::TNetwork::TDeviceHub_i>(localHub);

	auto collector = std::make_shared<TempCollector>();
	//STI::TNetwork::TDeviceCollection_i* tDeviceCollection = new STI::TNetwork::TDeviceCollection_i(collector);
	auto tDeviceCollection = std::make_shared<STI::TNetwork::TDeviceCollection_i>(collector);
	auto tDeviceCollection2 = std::make_shared<STI::TNetwork::TDeviceCollection_i>(collector);

	//PortableServer::Servant_var<STI::TNetwork::TDeviceCollection_i> tDeviceCollection = new STI::TNetwork::TDeviceCollection_i(collector);

	//PortableServer::ObjectId_var tDeviceCollectionID = orbMan->poa->activate_object(tDeviceCollection);
	PortableServer::ObjectId_var tDeviceCollectionID = poa->activate_object(tDeviceCollection.get());
	//PortableServer::ObjectId_var tDeviceCollectionID2 = poa->activate_object(tDeviceCollection2.get());


	//PortableServer::ObjectId_var tDeviceCollectionID2b = poa->activate_object(tDeviceCollection2.get());

	//CORBA::Object_var obj = 
	tDeviceCollection->_this();	//implicit activation
	tDeviceCollection2->_this();
	//tDeviceCollection->_remove_ref();


//	pman->activate();


	//can deactivate with stored ID or with poa
	//poa->deactivate_object(tDeviceCollectionID);
	poa->deactivate_object(*(poa->servant_to_id(tDeviceCollection.get())));
	//poa->deactivate_object(tDeviceCollectionID2);
	poa->deactivate_object(  *(poa->servant_to_id( tDeviceCollection2.get() )  ));
	
	//breaks
//	STI::TNetwork::TDeviceCollection_i* tmp = 0;
//	poa->deactivate_object(*(poa->servant_to_id(tmp)));


	tDeviceCollection = 0;
	tDeviceCollection2 = 0;

	//PortableServer::ObjectId_var tDeviceHubID = orbMan->poa->activate_object(deviceHubServant.get());
	//STI::TNetwork::TDeviceHub_var p = deviceHubServant->_this();
	//deviceHubServant->_remove_ref();

	orb->perform_work();
	//orbMan->run();

	orb->shutdown(false);
	orb->destroy();
	//orbMan->shutdown();
	//delete tDeviceCollection;


	// 
	return 0;
}
