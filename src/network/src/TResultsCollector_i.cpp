#include "TResultsCollector_i.h"

#include <sti/device/DeviceID.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/RawEvent.h>

#include "NetworkConvert.h"
#include "convert/Convert_Attribute.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ResultsCollector.h"

#include "RemoteFileServer.h"

#include <vector>
#include <memory>

using STI::TNetwork::TResultsCollector_i;
using STI::TNetwork::TShotID;
using STI::TNetwork::TEventEngineDependencyTree;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;
using STI::Device::Attribute;
using ::STI::TNetwork::TAttribute;
using STI::Engine::Measurement;
using ::STI::TNetwork::TMeasurement;
using STI::Engine::ShotID;
using ::STI::TNetwork::TShotID;
using STI::Utils::FileHolder;
using STI::Engine::ParsedDependencyTree;


TResultsCollector_i::TResultsCollector_i(STI::Engine::ResultsCollector* resultsCollector)
: resultsCollector(resultsCollector)
{
}

TResultsCollector_i::~TResultsCollector_i()
{
}


TShotID* TResultsCollector_i::getShotID()
{
	STI::TNetwork::TShotID_var tShotID(new STI::TNetwork::TShotID);

	if(resultsCollector != 0) {
		convert<ShotID, TShotID>(resultsCollector->getShotID(), tShotID);
	}

	return tShotID._retn();
}

::CORBA::Boolean TResultsCollector_i::addMeasurements(const ::STI::TNetwork::TDeviceID& deviceID, 
													  const ::STI::TNetwork::TMeasurementSeq& measurements,
													  ::STI::TNetwork::TFileServer_ptr sourceFileServer)
{
    if (resultsCollector != 0 && !CORBA::is_nil(sourceFileServer)) {

        auto newMeasurements = STI::Engine::MeasurementVector();
	    convert<TMeasurement, std::shared_ptr<Measurement>>(measurements, newMeasurements);

		STI::TNetwork::TFileServer_var sourceFileServer_var = STI::TNetwork::TFileServer::_duplicate(sourceFileServer);
		auto remoteFileServer = std::make_shared<STI::Network::RemoteFileServer>(sourceFileServer_var);

		return resultsCollector->addMeasurements(convert<TDeviceID, DeviceID>(deviceID), newMeasurements, remoteFileServer);
	}
    return false;
}

::CORBA::Boolean TResultsCollector_i::addAttributes(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TStringPairSeq& attributes)
{
    if (resultsCollector != 0) {

		std::map<std::string, std::string> remoteAttributes;

	    convert<::STI::TNetwork::TStringPairSeq, std::map<std::string, std::string>>(attributes, remoteAttributes);

		return resultsCollector->addAttributes(convert<TDeviceID, DeviceID>(deviceID), remoteAttributes);
	}
    return false;
}

