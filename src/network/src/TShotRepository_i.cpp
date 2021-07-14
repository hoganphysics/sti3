
#include "TShotRepository_i.h"
#include "ORBManager.h"
#include "Convert_EventEngine.h"
#include "deviceNet.h"

using STI::TNetwork::TShotRepository_i;
using STI::TNetwork::TMeasurementSeq;
using STI::TNetwork::TMeasurement;
using STI::Engine::Measurement;
using STI::Engine::MeasurementVector;
using STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Network::convert;



TShotRepository_i::TShotRepository_i(const std::shared_ptr<STI::Engine::ShotRepository>& shotRepository)
: shotRepository(shotRepository)
{
}

TShotRepository_i::~TShotRepository_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

::CORBA::Boolean TShotRepository_i::findShot(const ::STI::TNetwork::TShotID& sid)
{
    bool success = false;
	
	if (shotRepository != 0) {

        success = shotRepository->findShot(convert<TShotID, ShotID>(sid));
	}

	return success;
}

::CORBA::Boolean TShotRepository_i::getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TMeasurementSeq_out measurements)
{
    bool success = false;

	STI::TNetwork::TMeasurementSeq_var tMeasurementSeq_var(new STI::TNetwork::TMeasurementSeq);
	
	if (shotRepository != 0) {
        
        auto localMeasurements = std::make_shared<MeasurementVector>();
        success = shotRepository->getMeasurements(convert<TShotID, ShotID>(sid), localMeasurements);

		success &= convert<std::shared_ptr<Engine::Measurement>, TMeasurement>(*localMeasurements, tMeasurementSeq_var);
    
   		measurements = new STI::TNetwork::TMeasurementSeq();
		(*measurements) = tMeasurementSeq_var;
	}

	return success;
}

