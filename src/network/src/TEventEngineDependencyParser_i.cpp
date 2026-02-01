
#include "TEventEngineDependencyParser_i.h"

#include "convert/Convert_EventEngine.h"
#include "convert/Convert_DeviceTrace.h"

#include "NetworkConvert.h"
#include "EventEngineDependencyTree.h"
#include <sti/engine/EventEngineDependencyParser.h>
#include <sti/engine/EngineParsingMessage.h>


using STI::TNetwork::TEventEngineDependencyParser_i;
using STI::Network::convert;
using ::STI::TNetwork::TDeviceIDSeq;
using ::STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceTrace;
using STI::Device::DeviceID;
using ::STI::TNetwork::TDeviceID;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::Engine::EventEngineDependencyTree;


TEventEngineDependencyParser_i::TEventEngineDependencyParser_i(const std::shared_ptr<STI::Engine::EventEngineDependencyParser>& dependencyParser)
: localDependencyParser(dependencyParser)
{
}

TEventEngineDependencyParser_i::~TEventEngineDependencyParser_i()
{
}


void TEventEngineDependencyParser_i::getDependants(const TDeviceIDSeq& evtTargets, 
                        TEventEngineDependencyTree& tree, 
                        TDeviceIDSeq& missingTargets, 
						::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                        const TDeviceTrace& trace)
{
	messages = new STI::TNetwork::TEngineParsingMessageSeq();

    if (localDependencyParser != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

        std::set<DeviceID> missingIDs;
        std::set<DeviceID> targetIDs;
        convert<TDeviceID, DeviceID>(missingTargets, missingIDs);
        convert<TDeviceID, DeviceID>(evtTargets, targetIDs);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;

		localDependencyParser->getDependants(targetIDs, dependencyTree, missingIDs, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

		//convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);
        convert<DeviceID, TDeviceID>(missingIDs, missingTargets);
		
		
		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);

		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			(*messages) = tEngineParsingMessageSeq_var;
		}
	}
}

void TEventEngineDependencyParser_i::addDeviceEventTargets(TEventEngineDependencyTree& tree, 
													::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                                                    const TDeviceTrace& trace)
{
    if (localDependencyParser != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;

		localDependencyParser->addDeviceEventTargets(dependencyTree, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

        //convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);

		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);
		
		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			messages = new STI::TNetwork::TEngineParsingMessageSeq();
			(*messages) = tEngineParsingMessageSeq_var;
		}
	}
}


::CORBA::Boolean TEventEngineDependencyParser_i::ping()
{
	return true;
}

