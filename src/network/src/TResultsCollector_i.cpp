
#include "TResultsCollector_i.h"

#include "ORBManager.h"
#include "ResultsCollector.h"
#include "NetworkConvert.h"
#include "Convert_Attribute.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "RawEvent.h"
#include "DeviceID.h"

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
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}


TShotID* TResultsCollector_i::getShotID()
{
	STI::TNetwork::TShotID_var tShotID(new STI::TNetwork::TShotID);

	if(resultsCollector != 0) {
		convert<ShotID, TShotID>(resultsCollector->getShotID(), tShotID);
	}

	return tShotID._retn();
}

TEventEngineDependencyTree* TResultsCollector_i::getDependencies()
{
    // std::shared_ptr<ParsedDependencyTree> tree;

	STI::TNetwork::TEventEngineDependencyTree_var tTree(new STI::TNetwork::TEventEngineDependencyTree);

	if(resultsCollector != 0) {
        auto tree = resultsCollector->getDependencies();
        convert<std::shared_ptr<Engine::ParsedDependencyTree>, STI::TNetwork::TEventEngineDependencyTree>(tree, tTree);
	}

	return tTree._retn();   
}

void TResultsCollector_i::addEvents(const ::STI::TNetwork::TDeviceEventsSeq& parsedEvents)
{
    if (resultsCollector != 0) {

        STI::Engine::DeviceEventMap newEvents;
        convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(parsedEvents, newEvents);

		resultsCollector->addEvents(newEvents);
	}
}


void TResultsCollector_i::addTimingFiles(const ::STI::TNetwork::TFileHolderSeq& files)
{
    if (resultsCollector != 0) {

        std::vector<std::shared_ptr<FileHolder>> newFiles;
        convert<TNetwork::TFileHolderSeq, std::vector<std::shared_ptr<FileHolder>>>(files, newFiles);

		resultsCollector->addTimingFiles(newFiles);
	}
}

::CORBA::Boolean TResultsCollector_i::addMeasurements(const ::STI::TNetwork::TMeasurementSeq& measurements)
{
    if (resultsCollector != 0) {

        auto newMeasurements = std::make_shared<STI::Engine::MeasurementVector>();
	    convert<TMeasurement, std::shared_ptr<Measurement>>(measurements, *newMeasurements);

		return resultsCollector->addMeasurements(newMeasurements);
	}
    return false;
}

::CORBA::Boolean TResultsCollector_i::addAttributes(const ::STI::TNetwork::TDeviceID& deviceID, const ::STI::TNetwork::TAttributeSeq& attributes)
{
    if (resultsCollector != 0) {

	    std::vector<std::shared_ptr<Attribute>> remoteAttributes;
	    convert<TAttribute, std::shared_ptr<Attribute>>(attributes, remoteAttributes);

		return resultsCollector->addAttributes(convert<TDeviceID, DeviceID>(deviceID), remoteAttributes);
	}
    return false;
}

