
#include "TEventEngineScheduler_i.h"
#include "EventEngineScheduler.h"
#include "Device.h"
#include "DeviceTrace.h"
#include "DeviceID.h"
#include "NetworkConvert.h"
#include "ORBManager.h"
#include "EngineJobID.h"
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"
#include "Shot.h"

#include "EventEngineDependencyTree.h"
//#include "RemoteEventEngineJob.h"
#include "EngineParsingMessage.h"

#include <memory>


using STI::TNetwork::TEventEngineScheduler_i;
using ::STI::TNetwork::TDeviceIDSeq;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceTrace;
using STI::Device::DeviceID;
using STI::Engine::EventEngineScheduler;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::Engine::EventEngineDependencyTree;
using STI::Network::convert;
//using STI::Network::RemoteEventEngineJob;
using STI::Engine::EngineJobID;
using ::STI::TNetwork::TEngineJobID;
using STI::Engine::EventEngineJobType;
using STI::Engine::EventEngineJob;
using STI::Engine::Shot;

TEventEngineScheduler_i::TEventEngineScheduler_i(const std::shared_ptr<STI::Device::Device>& device)
{
   	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	device->getEngineScheduler(scheduler);

	engineScheduler = scheduler;
}

TEventEngineScheduler_i::~TEventEngineScheduler_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TEventEngineScheduler_i::parse(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TParsedShot_ptr shot)
{
	std::shared_ptr<Shot> parsedShot;
	bool success = convert<::STI::TNetwork::TParsedShot_ptr, std::shared_ptr<Shot>>(shot, parsedShot);
    
	if (engineScheduler != 0 && success) {

		engineScheduler->parse(convert<TParseID, STI::Engine::ParseID>(parseID), parsedShot);
	}
}

void TEventEngineScheduler_i::play(const ::STI::TNetwork::TShotID& shotID)
{
    if (engineScheduler != 0) {

		engineScheduler->play(convert<TShotID, STI::Engine::ShotID>(shotID));
	}
}


void TEventEngineScheduler_i::getDependants(const TDeviceIDSeq& evtTargets, 
                        TEventEngineDependencyTree& tree, 
                        TDeviceIDSeq& missingTargets, 
						::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                        const TDeviceTrace& trace)
{
    if (engineScheduler != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

        std::set<DeviceID> missingIDs;
        std::set<DeviceID> targetIDs;
        convert<TDeviceID, DeviceID>(missingTargets, missingIDs);
        convert<TDeviceID, DeviceID>(evtTargets, targetIDs);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;
		//convert<::STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(messages, generatedMessages);

		engineScheduler->getDependants(targetIDs, dependencyTree, missingIDs, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

		//convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);
        convert<DeviceID, TDeviceID>(missingIDs, missingTargets);
		
		
		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);

		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			messages = tEngineParsingMessageSeq_var.out();
		}
	}
}

void TEventEngineScheduler_i::addDeviceEventTargets(TEventEngineDependencyTree& tree, 
													::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                                                    const TDeviceTrace& trace)
{
    if (engineScheduler != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;

		engineScheduler->addDeviceEventTargets(dependencyTree, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

        //convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);

		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);
		
		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			messages = tEngineParsingMessageSeq_var.out();
		}
	}
}

// void TEventEngineScheduler_i::addJob(::STI::TNetwork::TEventEngineJob_ptr newJob)
// {
//     if (engineScheduler != 0) {

//         auto remoteJob = std::make_shared<RemoteEventEngineJob>(newJob);

// 		engineScheduler->addJob(remoteJob);
// 	}
// }

void TEventEngineScheduler_i::addJob(const ::STI::TNetwork::TEventEngineJob& newJob)
{


	// TEngineJobID jobID;
	// TDeviceID jobOwner;
	// TEngineJobStatus status;

	// TEngineID engineID;
	// TEventEngine eventEngine;

	// TParsedShot shot;
	// TEventEngineDependencyTree dependencies;
	// TDeviceIDSeq missingTargetIDs;


	



	if (engineScheduler != 0) {

		std::shared_ptr<EventEngineJob> remoteJob;

		convert<TNetwork::TEventEngineJob, std::shared_ptr<EventEngineJob>>(newJob, remoteJob);


		// EngineJobID jobID;
		// convert<TEngineJobID, EngineJobID>(newJob.jobID, jobID);

		// switch(jobID.type) {
		// 	case EventEngineJobType::Parse:
		// 		remoteJob = std::make_shared<LocalEventEngineJob>(
		// 			jobID.pid,
		// 			,
		// 			convert<TDeviceID, DeviceID>(newJob.jobOwner),

		// 		);
		// 	break;

		// 	case EventEngineJobType::Play:
		// 		remoteJob = std::make_shared<LocalEventEngineJob>(
		// 			jobID,
		// 			convert<TDeviceID, DeviceID>(newJob.jobOwner)
		// 		);
		// 	break;
		// }


		// //Parse jobs
		// LocalEventEngineJob(const ParseID& parseID, 
		// 					const std::shared_ptr<Shot>& shot,
		// 					const std::shared_ptr<EventEngineDependencyTree>& tree, 
		// 					const STI::Device::DeviceID& owner, 
		// 					const std::set<STI::Device::DeviceID>& missingTargets);

		// //Play jobs
		// LocalEventEngineJob(const EngineJobID& id, 
		// 					const STI::Device::DeviceID& owner);


		engineScheduler->addJob(remoteJob);
	}
}

void TEventEngineScheduler_i::cancelJob(const ::STI::TNetwork::TEngineJobID& jobID)
{
    if (engineScheduler != 0) {

		engineScheduler->cancelJob(convert<TEngineJobID, STI::Engine::EngineJobID>(jobID));
	}
}
