
#include "ORBManager.h"

#include <omniORB4/omniURI.h>

#include <sti/utils/utils.h>
#include "COSBindingNode.h"

#include <iostream>
#include <signal.h>
#include <cstring>

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

	running_ = false;
	blocking_ = false;

	for (unsigned i = 0; i < argc; i++) {
		delete[] argv[i];
	}
	delete[] argv;
}

ORBManager::~ORBManager()
{
	shutdown();
}

bool ORBManager::isPOAactive() const 
{ 
	return (!CORBA::is_nil(poa)) && poa_is_active; 
}

PortableServer::POA_ptr ORBManager::getPOA() const
{
	return PortableServer::POA::_duplicate(poa);
}

bool ORBManager::running()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	return running_;
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

		if (running_) {
			return;
		}
		running_ = true;
	}

	orb->perform_work();
//	orb->run();
}

bool ORBManager::blocking()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	return blocking_;
}

void ORBManager::block()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);

	if (blocking_) return;

	blocking_ = true;

	signal(SIGINT, ORBManager::signal_callback_handler);

	//ignore SIGPIPE signals
	signal(13, SIG_IGN);	//Note SIGPIPE=13 in linux; not defined in windows
	// signal(SIGPIPE, signal_callback_handler);	//ignore SIGPIPE signals

	while (blocking_) {
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
	blocking_ = false;
	wakeCondition.notify_all();
}


void ORBManager::shutdown()
{
	std::unique_lock<std::mutex> writeLock(orbMutex);
	if (running_ && orb_initialized)
	{
		running_ = false;
		orb_initialized = false;
		poa_is_active = false;
		std::cerr << "Shutting down ORB" << std::endl;

		orb->shutdown(true);

		//ignore SIGPIPE signals
		signal(13, SIG_IGN);	//Note SIGPIPE=13 in linux; not defined in windows
		// signal(SIGPIPE, signal_callback_handler);	//ignore SIGPIPE signals

		orb->destroy();
		// orb->destroy() causes error
		// try {
		// 	orb->destroy();
		// }
		// catch(...) {
		// 	std::cerr << "Caught exception on orb->destroy()" << std::endl;
		// }
		
	}
}

std::string ORBManager::printNameTree(const std::string& baseContext) const
{
	CosNaming::NamingContext_var baseContextVar;
	getNamingContext(baseContext, baseContextVar);
	CosNaming::NamingContext_var base(baseContextVar);

	COSBindingNode node(baseContext, base);

	return node.printTree();
}

void ORBManager::getObjectContexts(const std::string& baseContext, const std::string& objectName,
											std::vector<std::string>& objContexts)
{
	CosNaming::NamingContext_var baseContextVar;

	getNamingContext(baseContext, baseContextVar);
	CosNaming::NamingContext_var base(baseContextVar);

	COSBindingNode node(baseContext, base);
	node.getLiveLeafs(objectName, objContexts);
}

void ORBManager::getAllLiveObjectContexts(const std::string& baseContext, const std::string& objectName,
											std::vector<std::string>& objContexts)
{
	getObjectContexts(baseContext, objectName, objContexts);
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

		if (!getRootContext(rootContext)) return false;

		contextBase = CosNaming::NamingContext::_narrow(rootContext->resolve(contextName));

		success = true;
	}
	catch (CORBA::Exception&)
	{
		success = false;
		//std::cerr << "NamingContext exception." << std::endl;
	}
	catch (...) {
		success = false;
		//std::cerr << "Unspecified exception caught when attempting getNamingContext(" << context << ")" << std::endl;
	}

	return success;
}


bool ORBManager::bindObjectReference(const std::string& objectFullPath, CORBA::Object_ptr objref)
{
	return bindObjectReference(objectFullPath, objref, std::cerr);
}

bool ORBManager::bindObjectReference(const std::string& objectFullPath, CORBA::Object_ptr objref, std::ostream& errorBuf)
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
				obj = context->resolve(contextName);
				context = CosNaming::NamingContext::_narrow(obj);
			}
			catch (CosNaming::NamingContext::NotFound&)
			{
				try {
					// Create the context only when it is absent. omniNames logs
					// bind_new_context even when the following bind reports
					// AlreadyBound, so trying to create first grows its redo log.
					context = context->bind_new_context(contextName);
				}
				catch (CosNaming::NamingContext::AlreadyBound&) {
					// Another client created the context after our resolve.
					obj = context->resolve(contextName);
					context = CosNaming::NamingContext::_narrow(obj);
				}
			}

			if (CORBA::is_nil(context))
			{
				errorBuf << "Error: ORBManager::bindObjectReference failed to narrow naming context." << std::endl;
				return false;
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
			CORBA::Object_var existing = context->resolve(objectName);

			// omniNames persists every rebind as a destroy/bind pair. Avoid
			// rewriting an unchanged reference during periodic self-rebind.
			CORBA::String_var existingIor = orb->object_to_string(existing);
			CORBA::String_var replacementIor = orb->object_to_string(objref);
			if (std::strcmp(existingIor.in(), replacementIor.in()) != 0) {
				context->rebind(objectName, objref);
			}
		}
	}
	catch (CORBA::TRANSIENT& ex)
	{
		errorBuf << "Caught system exception CORBA::"
			<< ex._name() << " -- unable to contact the "
			<< "naming service." << std::endl
			<< "Make sure the naming server is running and that omniORB is "
			<< "configured correctly." << std::endl;

		return false;
	}
	catch (CORBA::SystemException& ex)
	{
		errorBuf << "Caught a CORBA::" << ex._name()
			<< " while using the naming service." << std::endl;
		return false;
	}

	return true;
}


bool ORBManager::unbindObjectReference(const std::string& objectFullPath)
{
	if (objectFullPath.empty() || objectFullPath.find("//") != std::string::npos) {
		return false;
	}

	CosNaming::NamingContext_var rootContext;
	if (!getRootContext(rootContext)) {
		return false;
	}

	try {
		CosNaming::Name_var objectName = omni::omniURI::stringToName(objectFullPath.c_str());
		rootContext->unbind(objectName);
		return true;
	}
	catch (CosNaming::NamingContext::NotFound&) {
	}
	catch (CosNaming::NamingContext::CannotProceed&) {
	}
	catch (CosNaming::NamingContext::InvalidName&) {
	}
	catch (CORBA::Exception&) {
	}

	return false;
}

bool ORBManager::getObjectReference(const std::string& objectFullPath, CORBA::Object_var& objref)
{
	return getObjectReference(objectFullPath, objref, std::cerr);
}

bool ORBManager::getObjectReference(const std::string& objectFullPath, CORBA::Object_var& objref, std::ostream& errorBuf)
{
	bool success = false;

	std::size_t nullPos = objectFullPath.find("//");	//look for any null contexts in path (empty string betweeen '/' separators)

	if (nullPos != std::string::npos) {
		return false;
	}

	CosNaming::NamingContext_var rootContext;
	if (!getRootContext(rootContext)) {
		return false;
	}

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
		errorBuf << "Error: Caught CORBA::" << ex._name()
			<< " when trying to resolve Object '" << objectFullPath << "'." << std::endl 
			<< "The Object and/or Context was not found." << std::endl;
	}
	catch (CORBA::TRANSIENT& ex) {
		errorBuf << "Caught system exception CORBA::" << ex._name()
			<< " -- unable to contact the naming service." << std::endl
			<< "Make sure the naming server is running and that omniORB is configured correctly." << std::endl;
	}
	catch (CORBA::SystemException& ex) {
		errorBuf << "Caught a CORBA::" << ex._name()
			<< " while using the naming service." << std::endl;
	}

	return success;
}
