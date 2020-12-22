
#include "RemoteEventEngineJob.h"


using STI::Network::RemoteEventEngineJob;


RemoteEventEngineJob::RemoteEventEngineJob(::STI::TNetwork::TEventEngineJob_ptr engineJob)
	: tEventEngineJob(STI::TNetwork::TEventEngineJob::_duplicate(engineJob))
{
}

RemoteEventEngineJob::~RemoteEventEngineJob()
{
}

STI::Engine::EngineJobID RemoteEventEngineJob::getJobID() const
{
}

STI::Device::DeviceID RemoteEventEngineJob::getJobOwner() const
{
}

EngineJobStatus RemoteEventEngineJob::getStatus() const
{
}


void RemoteEventEngineJob::markRunning(const  STI::Engine::EngineID& id)
{
}

void RemoteEventEngineJob::markComplete()
{
}

void RemoteEventEngineJob::markCancelled()
{
}


const  STI::Engine::EngineID& RemoteEventEngineJob::getEngineID() const
{
}

bool RemoteEventEngineJob::getEngine(std::shared_ptr< STI::Engine::EventEngine>& eventEngine) const
{
   	bool success = false;

	try {
		success = tEventEngineJob->getEngine(...);	//remote call
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

void RemoteEventEngineJob::setEventEngine(const std::shared_ptr< STI::Engine::EventEngine>& eventEngine)
{
}

bool RemoteEventEngineJob::getParsedShot(std::shared_ptr< STI::Engine::ParsedShot>& shot) const
{
    return false;
}

bool RemoteEventEngineJob::getDependencies(std::shared_ptr< STI::Engine::EventEngineDependencyTree>& tree) const
{
    return false;
}


std::set<STI::Device::DeviceID> RemoteEventEngineJob::getMissingTargetIDs() const
{
    bool success = false;

	try {
		tEventEngineJob->getMissingTargetIDs();	//remote call
        success = true;
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
}
