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

#include <sti/utils/Configuration.h>

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
	// ORBManager(const std::string& nameServiceIP, const std::string& args);
	ORBManager(const std::string& args);
	friend class Concrete_ORBManager;

public:

	virtual ~ORBManager();
	
	static std::shared_ptr<ORBManager> getInstance();
	static std::shared_ptr<ORBManager> getInstance(const STI::Utils::Configuration& orbConfig, const std::string& args);
	// static std::shared_ptr<ORBManager> getInstance(const std::string& nameServiceIP, const std::string& args);
	
	// static void setOptions(const STI::Utils::Configuration& config);

	bool running();
	
	void run();
	void shutdown();
	void block();
	void unblock();

	std::string printNameTree(const std::string& baseContext) const;

	void getAllLiveObjectContexts(const std::string& baseContext, const std::string& objectName, std::vector<std::string>& objContexts);

	bool bindObjectReference(const std::string& objectFullPath, CORBA::Object_ptr objref);
	bool unbindObjectReference(const std::string& objectFullPath);	
	bool getObjectReference(const std::string& objectFullPath, CORBA::Object_ptr& objref);

	// static void activateServant2(PortableServer::Servant p_servant);
	// static void activateServant3(PortableServer::Servant p_servant);
	// static void activateServant3(PortableServer::ServantBase& servant);

	static void activateServant(PortableServer::ServantBase& servant);
	// static void activateServant(PortableServer::Servant p_servant);
	static void deactivateServant(PortableServer::Servant p_servant);

private:

	static bool orb_initialized;
	static std::mutex orbInitMutex;
	static std::shared_ptr<ORBManager> instance;
	static STI::Utils::Configuration omniOptions;

	bool getRootContext(CosNaming::NamingContext_var& context) const;
	//CosNaming::NamingContext_ptr getNamingContext(const std::string& context) const;
	bool getNamingContext(const std::string& context, CosNaming::NamingContext_var& contextBase) const;

	CORBA::ORB_var orb;
	PortableServer::POAManager_var poa_manager;
	PortableServer::POA_var root_poa;
	PortableServer::POA_var poa;

	bool _running;
	bool _blocking;
	bool poa_is_active;

	mutable std::mutex orbMutex;
	mutable std::condition_variable wakeCondition;

	static void signal_callback_handler(int signum);
	
};


} //Network
} //STI


#endif

