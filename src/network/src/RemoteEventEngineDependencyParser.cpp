
#include "RemoteEventEngineDependencyParser.h"

#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include "convert/Convert_DeviceTrace.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"

#include <sti/device/DeviceTrace.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/RawEvent.h>


using STI::Network::RemoteEventEngineDependencyParser;
using STI::Network::convert;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEventEngineDependencyTree;
using STI::Device::DeviceTrace;
using STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::TNetwork::TEngineParsingMessage;
using STI::Engine::EngineParsingMessage;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TEventEngineDependencyParser;


RemoteEventEngineDependencyParser::RemoteEventEngineDependencyParser(::STI::TNetwork::TEventEngineDependencyParser_var dependencyParser)
: TReferenceHolder<TEventEngineDependencyParser>(dependencyParser)
{
}

RemoteEventEngineDependencyParser::~RemoteEventEngineDependencyParser()
{
}

void RemoteEventEngineDependencyParser::getDependants(const std::set<STI::Device::DeviceID>& evtTargets, 
											   STI::Engine::EventEngineDependencyTree& tree, 
                                			   std::set<STI::Device::DeviceID>& missingTargets, 
											   std::vector<STI::Engine::EngineParsingMessage>& messages, 
											   const STI::Device::DeviceTrace& trace)
{
	std::unique_lock<std::mutex> dependencyLock(dependencyMutex);

	if (isDisabled()) return;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {

        STI::TNetwork::TDeviceIDSeq_var tEvtTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(evtTargets, tEvtTargets);

        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);
    
        STI::TNetwork::TDeviceIDSeq_var tMissingTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(missingTargets, tMissingTargets);

		getTRef()->getDependants(tEvtTargets, tTree, tMissingTargets, tEngineParsingMessages,
                                             convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
        convert<TDeviceID, DeviceID>(tMissingTargets, missingTargets);
		convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParsingMessages, messages);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteEventEngineDependencyParser::addDeviceEventTargets(EventEngineDependencyTree& tree, 
													   std::vector<EngineParsingMessage>& messages, 
													   const DeviceTrace& trace)
{
	std::unique_lock<std::mutex> dependencyLock(dependencyMutex);

	if (isDisabled()) return;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {
        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);

		getTRef()->addDeviceEventTargets(tTree, tEngineParsingMessages, 
                                                     convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
		convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParsingMessages, messages);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteEventEngineDependencyParser::ping() const
{
	std::unique_lock<std::mutex> dependencyLock(dependencyMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

