
#include "RemoteShotRepository.h"
#include "Convert_ResultTicket.h"
#include "Convert_EventEngine.h"
#include "orbTypes.h"


using STI::Network::RemoteShotRepository;
using STI::TNetwork::TReferenceHolder;
using STI::Network::convert;
using ::STI::TNetwork::TShotRepository;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::Engine::MeasurementVector;
using STI::Engine::ParseTicket;
using STI::Engine::Measurement;


RemoteShotRepository::RemoteShotRepository(::STI::TNetwork::TShotRepository_ptr repo)
: TReferenceHolder<TShotRepository>(repo, shotRepoMutex)
{
}

RemoteShotRepository::~RemoteShotRepository()
{
}

bool RemoteShotRepository::findShot(const STI::Engine::ShotID& sid)
{
	std::unique_lock<std::mutex> repoLock(shotRepoMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;

	try {

		success = getTRef()->findShot(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success;
}

bool RemoteShotRepository::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements)
{
	std::unique_lock<std::mutex> repoLock(shotRepoMutex);
	
    if (isDisabled()) return false;
    
    bool success = false;
   	STI::TNetwork::TMeasurementSeq_var tMeasurements(new STI::TNetwork::TMeasurementSeq);

	try {

		success = getTRef()->getMeasurements(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), tMeasurements);	//remote call

        if (success) {
            measurements = std::make_shared<MeasurementVector>();
            success &= convert<STI::TNetwork::TMeasurement, std::shared_ptr<Measurement>>(tMeasurements, *measurements);
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

    return success && (measurements != 0);

}

bool RemoteShotRepository::getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket)
{
	// std::unique_lock<std::mutex> repoLock(shotRepoMutex);
	
    // if (isDisabled()) return false;
    
    // bool success = false;
   	// STI::TNetwork::TParseTicket_var tParseTicket(new STI::TNetwork::TParseTicket);

	// try {

	// 	success = getTRef()->getParseTicket(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(sid), tParseTicket);	//remote call

    //     if (success) {
    //         success &= convert<STI::TNetwork::TParseTicket, std::shared_ptr<ParseTicket>>(tParseTicket, parseTicket);
    //     }
	// }
	// catch (CORBA::TRANSIENT&) {
	// }
	// catch (CORBA::SystemException&) {
	// }
	// catch (CORBA::Exception&) {
	// }

    // return success && (parseTicket != 0);
    return false;
}

