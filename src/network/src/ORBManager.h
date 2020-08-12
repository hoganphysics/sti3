#ifndef STI_NETWORK_ORBMANAGER_H
#define STI_NETWORK_ORBMANAGER_H

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB4/CORBA.h>
#endif

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace STI
{
namespace Network
{

class HubID;
class NetworkDeviceHubWrapper;
class COSBindingNode;

class Concrete_ORBManager;

class ORBManager
{
private:

	//This class is a singleton, so that only one instance of ORB is created.
	ORBManager(const std::string& nameServiceIP, const std::string& args);
	friend class Concrete_ORBManager;

public:
	
	virtual ~ORBManager();

	bool running();
	
	void run();
	void shutdown();
	void block();
	void unblock();

//	bool registerHub(const HubID& hubID, const std::shared_ptr<NetworkDeviceHubWrapper>& deviceHub);


	void getAllLiveObjectContexts(const std::string& baseContext, const std::string& objectName, std::vector<std::string>& objContexts);

	bool bindObjectReference(const std::string& objectFullPath, CORBA::Object_ptr objref);
	bool getObjectReference(const std::string& objectFullPath, CORBA::Object_ptr& objref);

	static std::shared_ptr<ORBManager> getInstance(const std::string& nameServiceIP, const std::string& args);

	static void deactivateServant(PortableServer::Servant p_servant);

private:

	static bool orb_initialized;
	static std::shared_ptr<ORBManager> instance;

	bool getRootContext(CosNaming::NamingContext_var& context);
	CosNaming::NamingContext_ptr getNamingContext(const std::string& context);

	CORBA::ORB_var orb;
	PortableServer::POAManager_var poa_manager;
	PortableServer::POA_var poa;

	bool _running;
	bool _blocking;

	//std::unique_ptr<COSBindingNode> bindingTree;

	mutable std::mutex orbMutex;
	mutable std::condition_variable wakeCondition;

	static void signal_callback_handler(int signum);

	
};


} //Network
} //STI


#endif

