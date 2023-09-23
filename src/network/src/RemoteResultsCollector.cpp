#include "RemoteResultsCollector.h"

#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/Measurement.h>

#include "EventEngineDependencyTree.h"
#include "TFileServerRefInterface.h"

#include "NetworkConvert.h"
#include "convert/Convert_Attribute.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ResultsCollector.h"
#include "generated/orbTypes.h"


using STI::Network::RemoteResultsCollector;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TResultsCollector;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEventEngineDependencyTree;
using STI::Network::TFileServerRefInterface;


RemoteResultsCollector::RemoteResultsCollector(::STI::TNetwork::TResultsCollector_ptr collector)
: TReferenceHolder<TResultsCollector>(collector, collectorMutex)
{
}

RemoteResultsCollector::~RemoteResultsCollector()
{
}

STI::Engine::ShotID RemoteResultsCollector::getShotID() const
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

    STI::Engine::ShotID sid;
	
    if (isDisabled()) {
        return sid; //empty
    }

	try {
		auto tShotID = getTRef()->getShotID();	//remote call

        if (tShotID != 0) {
            sid = convert<STI::TNetwork::TShotID, STI::Engine::ShotID>(*tShotID);            
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    return sid;
}

bool RemoteResultsCollector::addMeasurements(const STI::Device::DeviceID& deviceID, 
											 const STI::Engine::MeasurementVector& measurements, 
											 const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer)
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

	if (isDisabled()) return false;
    
    bool success = false;

    STI::TNetwork::TFileServer_var tFileServer;

    if (!TFileServerRefInterface::getTFileServerReference(sourceFileServer, tFileServer)) {
        return false;
    }

    STI::TNetwork::TMeasurementSeq_var tMeasurements(new STI::TNetwork::TMeasurementSeq);

	try {
		convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(measurements, tMeasurements);

		success = getTRef()->addMeasurements(
							convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceID), 
							tMeasurements,
							tFileServer);	//remote call
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

bool RemoteResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes)
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

	if (isDisabled()) return false;

    bool success = false;
    
    STI::TNetwork::TStringPairSeq_var tAttributes(new STI::TNetwork::TStringPairSeq);

	try {
		convert<std::map<std::string, std::string>, STI::TNetwork::TStringPairSeq>(attributes, tAttributes);

		success = getTRef()->addAttributes(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceID), tAttributes);	//remote call
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

