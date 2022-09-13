
#include "ORBManager.h"

#include <omniORB4/omniURI.h>

#include <sti/utils/utils.h>
#include "COSBindingNode.h"

#include <iostream>
#include <signal.h>
#include <string.h>

using STI::Network::ORBManager;
using STI::Network::COSBindingNode;



// Device reference name registered on the NameService:
// "TRemoteDevice.Object"

// Server reference name:
// "STI/Network/TServer.Object"

/*

STI/MAGIS/TDeviceHub.Object
STI/AtomSource1/TDeviceHub.Object
STI/MAGIS/AtomSource1/TDeviceHub.Object

---------Example config files:

Name = MAGIS
Address = 192.168.1.1
Module = 0
Target server = root
NameServer IP:Port = 192.168.1.1:2809

Nameservice:
--> STI/MAGIS/TDeviceHub.Object
--> STI/192.168.1.1/0/MAGIS/TDeviceHub.Object
--> STI/192.168.1.1/MAGIS/TDeviceHub.Object	**** Hubs don't have modules

Name = AtomSource1
Address = 192.168.1.2
Module = 0
Target server = 192.168.1.1/0/MAGIS
NameServer IP:Port = 192.168.1.1:2809

Nameservice:
--> STI/192.168.1.2/0/AtomSource1/TDeviceHub.Object							(server can't find)
--> STI/192.168.1.1/0/MAGIS/192.168.1.2/0/AtomSource1/TDeviceHub.Object		(devices can't find)
Consider doing both

Nameservice:
--> STI/192.168.1.2/AtomSource1/TDeviceHub.Object
--> STI/192.168.1.1/MAGIS/192.168.1.2/AtomSource1/TDeviceHub.Object

Name = DDS
Address = 192.168.1.4
Module = 0
target server = 192.168.1.2/AtomSource1
NameServer IP:Port = 192.168.1.1:2809
--> STI/192.168.1.4/9/LocalHub/TDeviceHub.Object


Name = Digital Out
Address = 192.168.1.4
Module = 0
target server = MAGIS/AtomSource1
NameServer IP:Port = 192.168.1.1:2809


Name = DDS
Address = 192.168.1.8
Module = 0
target server = MAGIS/AtomSource2
NameServer IP:Port = 192.168.1.1:2809

*/

//namespace {

//class Concrete_ORBManager : public ORBManager
//{
//public:
//	Concrete_ORBManager(const std::string& nameServiceIP, const std::string& args) 
//		: ORBManager(nameServiceIP, args) {}
//	~Concrete_ORBManager() {}
//
//};

//struct Concrete_ORBManager : public ORBManager {};


//} // anonymous

namespace STI
{
namespace Network
{

class Concrete_ORBManager : public ORBManager
{
public:

	Concrete_ORBManager(const std::string& args) 
		: ORBManager(args) {}
};

} // Network
} // STI

bool ORBManager::orb_initialized = false;
std::shared_ptr<ORBManager> ORBManager::instance = 0;
std::mutex ORBManager::orbInitMutex = std::mutex();
STI::Utils::Configuration ORBManager::omniOptions = STI::Utils::Configuration();


std::shared_ptr<ORBManager> ORBManager::getInstance(const STI::Utils::Configuration& orbConfig, const std::string& args)
{
	std::unique_lock<std::mutex> writeLock(orbInitMutex);

	if (!orb_initialized) {

		omniOptions = orbConfig;
		instance = std::make_shared<STI::Network::Concrete_ORBManager>(args);
		//orb_initialized = true;
	}
	return instance;
}

std::shared_ptr<ORBManager> ORBManager::getInstance()
{
	return getInstance(omniOptions, "");
}

bool ORBManager::orbInstanceInitializd()
{
	std::unique_lock<std::mutex> writeLock(orbInitMutex);
	return orb_initialized;
}

ORBManager::ORBManager(const std::string& args)
{
	poa_is_active = false;
	
	auto paramsNames = omniOptions.getParameterNames();
	
	//Prepare ORB arguments
	std::vector<std::string> arguments;
	//STI::Utils::splitString(args, " ", arguments);

	std::string prefix = "-ORB";
	for (auto& name : paramsNames) {
		auto values = omniOptions.getList(name);	//possible multiple entries per name

		for (auto& value : values) {
			arguments.push_back(prefix + name);
			arguments.push_back(value);
		}
	}

	//Initialize argv
	int argc = static_cast<int>(arguments.size());
	char** argv = new char*[argc];
	for (unsigned i = 0; i < argc; i++) {
		argv[i] = new char[arguments[i].size() + 1];
		// strcpy_s(argv[i], arguments[i].size() + 1, arguments[i].c_str());
		strcpy(argv[i], arguments[i].c_str());
	}

	const char* options2[][2] = { { 0, 0 } };

	//Initialize ORB
	try {
		orb = CORBA::ORB_init(argc, argv, "", options2);	//(const char* (*)[2]) 
	
		CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA");
		
		root_poa = PortableServer::POA::_narrow(poa_obj);
		poa_manager = root_poa->the_POAManager();

		// poa = PortableServer::POA::_narrow(poa_obj);
		// poa_manager = poa->the_POAManager();

		// poa_manager->_NP_is_nil();

		poa_manager->activate();
		poa_is_active = true;


		//Create POA with a Bidirectional policy
		CORBA::PolicyList policies;
		policies.length(1);
		CORBA::Any a;
		a <<= BiDirPolicy::BOTH;
		policies[0] = orb->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);

		poa = root_poa->create_POA("bidir", poa_manager, policies);

		orb_initialized = true;

	}
	catch (CORBA::SystemException& ex) {
		std::cerr << "Caught CORBA::" << ex._name()
			<< " when trying to initialize ORBManager." << std::endl;
	}
	catch (CORBA::Exception& ex) {
		std::cerr << "Caught CORBA::Exception: " << ex._name()
			<< " when trying to initialize ORBManager." << std::endl;
	}
	catch (omniORB::fatalException& fe) {
		std::cerr << "Caught omniORB::fatalException "
			<< "when trying to initialize ORBManager:" << std::endl;
		std::cerr << "  file: " << fe.file() << std::endl;
		std::cerr << "  line: " << fe.line() << std::endl;
		std::cerr << "  mesg: " << fe.errmsg() << std::endl;
	}

	_running = false;
	_blocking = false;

	for (unsigned i = 0; i < argc; i++) {
		delete[] argv[i];
	}
	delete[] argv;
}



ORBManager::~ORBManager()
{
	shutdown();
}

void ORBManager::activateServant(PortableServer::ServantBase& servant)
{
	std::shared_ptr<ORBManager> orbManager = ORBManager::instance;

	if (orbManager != 0 && !(CORBA::is_nil(orbManager->poa)) && orbManager->poa_is_active) {
		
		orbManager->poa->activate_object(&servant);
	}
}

void ORBManager::deactivateServant(PortableServer::Servant p_servant)
{
	std::shared_ptr<ORBManager> orbManager = ORBManager::instance;

	if (orbManager != 0 && !(CORBA::is_nil(orbManager->poa)) && orbManager->poa_is_active) {

		try {
			auto objref = (orbManager->poa->servant_to_id(p_servant));

			if (objref != 0) {

				orbManager->poa->deactivate_object(*objref);
			}	
		}
		catch (PortableServer::POA::ServantNotActive& e) {
		}

	}
}

bool ORBManager::running()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	return _running;
}

bool ORBManager::initialized()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	return orb_initialized;
}

void ORBManager::run()
{
	{
		std::unique_lock<std::mutex> writeLock(orbMutex);
		
		if (!orb_initialized) {
			std::cerr << "Error: ORB not initialized. Aborting ORBManager::run()" << std::endl;
			return;
		}

		if (_running) {
			return;
		}
		_running = true;
	}

	orb->perform_work();
//	orb->run();
}

void ORBManager::block()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	_blocking = true;

	signal(SIGINT, ORBManager::signal_callback_handler);

	while (_blocking) {
		wakeCondition.wait(writeLock);
	}
}

void ORBManager::signal_callback_handler(int signum)
{
	std::cout << std::endl;
	std::cout << "Caught signal: " << signum << std::endl;

	//Caught control-C:  Stop blocking
	ORBManager::instance->unblock();
}

void ORBManager::unblock()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	_blocking = false;
	wakeCondition.notify_all();
}


void ORBManager::shutdown()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	if (_running && orb_initialized)
	{
		_running = false;
		orb_initialized = false;
		poa_is_active = false;
		std::cerr << "Shutting down ORB" << std::endl;

		orb->shutdown(true);
		orb->destroy();

	}
}

std::string ORBManager::printNameTree(const std::string& baseContext) const
{
	//CosNaming::NamingContext_var base(getNamingContext(baseContext));

	CosNaming::NamingContext_var baseContextVar;
	getNamingContext(baseContext, baseContextVar);
	CosNaming::NamingContext_var base(baseContextVar);

	COSBindingNode node(baseContext, base);

	return node.printTree();
}

void ORBManager::getAllLiveObjectContexts(const std::string& baseContext, const std::string& objectName, 
											std::vector<std::string>& objContexts)
{
	CosNaming::NamingContext_var baseContextVar;

	//CosNaming::NamingContext_var base(getNamingContext(baseContext));
	getNamingContext(baseContext, baseContextVar);
	CosNaming::NamingContext_var base(baseContextVar);

	COSBindingNode node(baseContext, base);
	node.prune();
	
	node.getLiveLeafs(objectName, objContexts);
}


bool ORBManager::getRootContext(CosNaming::NamingContext_var& context) const
{
	//Obtains the root context of the Name Service

	bool success = false;
	
	try {
		
		CORBA::Object_var obj = orb->resolve_initial_references("NameService");

		context = CosNaming::NamingContext::_narrow(obj);		// Narrow the reference to a Context.

		success = true;

		if (CORBA::is_nil(context)) {
			std::cerr << "Failed to narrow the root naming context." << std::endl;
			success = false;
		}
	}
	catch (CORBA::NO_RESOURCES&) {
		std::cerr << "Caught NO_RESOURCES exception. You must configure omniORB "
			<< "with the location" << std::endl << "of the Naming Service." << std::endl;
	}
	catch (CORBA::ORB::InvalidName&) {
	//	// This should not happen!
		std::cerr << "Service required is invalid [does not exist]." << std::endl;
	}
	catch (CORBA::TRANSIENT& ex) {
		std::cerr << "Caught system exception CORBA::" << ex._name()
			<< std::endl << " when attempting to contact the "
			<< "Name Service." << std::endl;
	}
	catch (CORBA::SystemException& ex) {
		std::cerr << "Caught a CORBA::" << ex._name()
			<< " while using the naming service." << std::endl;
	}

	return success;
}


bool ORBManager::getNamingContext(const std::string& context, CosNaming::NamingContext_var& contextBase) const
{
	CosNaming::NamingContext_var rootContext;

	bool success = false;

	try {
		CosNaming::Name_var contextName;

		contextName = omni::omniURI::stringToName(context.c_str());

		getRootContext(rootContext);
		contextBase = CosNaming::NamingContext::_narrow(rootContext->resolve(contextName));

		success = true;
	}
	catch (CORBA::Exception&)
	{
		success = false;
		std::cerr << "NamingContext exception." << std::endl;
	}
	catch (...) {
		success = false;
		std::cerr << "Unspecified exception caught when attempting getNamingContext(" << context << ")" << std::endl;
	}

	return success;
}


bool ORBManager::bindObjectReference(const std::string& objectFullPath, CORBA::Object_ptr objref)
{
	CORBA::Object_var obj;
	CosNaming::NamingContext_var context;
	CosNaming::Name_var contextName;
	CosNaming::Name_var objectName;

	// Split object full name into a list of the form {Context, Context, ..., Context, Object}
	std::vector<std::string> tokens;
	STI::Utils::splitString(objectFullPath, "/", tokens);

	if (!getRootContext(context)) {
		return false;
	}

	// Bind all the contexts to the Root Context
	try {
		// Sequentially binds a context with name tokens[i] to the previous context
		for (unsigned i = 0; i < tokens.size() - 1; ++i) {	//skip the last token (the object name)

			contextName = omni::omniURI::stringToName(tokens.at(i).c_str());

			try {
				// Bind the context to the previous context
				context = context->bind_new_context(contextName);
			}
			catch (CosNaming::NamingContext::AlreadyBound&)
			{
				// If the context already exists, this exception will be raised.
				// In this case, just resolve the name and assign context to the object.

				obj = context->resolve(contextName);
				context = CosNaming::NamingContext::_narrow(obj);

				if (CORBA::is_nil(context))
				{
					std::cerr << "Failed to narrow naming context." << std::endl;
					return false;
				}
			}
		}

		// Now bind the object to the last context
		// The last token is the object name
		objectName = omni::omniURI::stringToName(tokens.back().c_str());

		try {
			context->bind(objectName, objref);
		}
		catch (CosNaming::NamingContext::AlreadyBound&)
		{
			// rebind() will overwrite any Object previously bound to context/objectName

			context->rebind(objectName, objref);
		}
	}
	catch (CORBA::TRANSIENT& ex)
	{
		std::cerr << "Caught system exception CORBA::"
			<< ex._name() << " -- unable to contact the "
			<< "naming service." << std::endl
			<< "Make sure the naming server is running and that omniORB is "
			<< "configured correctly." << std::endl;

		return false;
	}
	catch (CORBA::SystemException& ex)
	{
		std::cerr << "Caught a CORBA::" << ex._name()
			<< " while using the naming service." << std::endl;
		return false;
	}

	return true;
}


bool ORBManager::unbindObjectReference(const std::string& objectFullPath)
{

	// CosNaming::NamingContext_var base(getNamingContext(baseContext));

	return false;
}


bool ORBManager::getObjectReference(const std::string& objectFullPath, CORBA::Object_ptr& objref)
{
	bool success = false;

	CosNaming::NamingContext_var rootContext;
	getRootContext(rootContext);

	CosNaming::Name_var objectName =
		omni::omniURI::stringToName(objectFullPath.c_str());

	try {
		// Resolve the name to an object reference.
		objref = rootContext->resolve(objectName);
		success = true;
	}
	catch (CosNaming::NamingContext::NotFound& ex) {
		// This exception is thrown if any of the components of the
		// path [contexts or the object] aren't found:
		std::cerr << "Error: Caught CORBA::" << ex._name()
			<< " when trying to resolve Object '" << objectFullPath << "'." << std::endl 
			<< "The Object and/or Context was not found." << std::endl;
	}
	catch (CORBA::TRANSIENT& ex) {
		std::cerr << "Caught system exception CORBA::" << ex._name() 
			<< " -- unable to contact the naming service." << std::endl
			<< "Make sure the naming server is running and that omniORB is configured correctly." << std::endl;
	}
	catch (CORBA::SystemException& ex) {
		std::cerr << "Caught a CORBA::" << ex._name()
			<< " while using the naming service." << std::endl;
	}

	return success;
}
