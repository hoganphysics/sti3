
#include "RemoteResultsCollector.h"
#include "ShotID.h"
#include "EventEngineDependencyTree.h"
#include "ParsedDependencyTree.h"

#include "NetworkConvert.h"
#include "Convert_Attribute.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "orbTypes.h"


using STI::Network::RemoteResultsCollector;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TResultsCollector;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEventEngineDependencyTree;


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


std::shared_ptr<STI::Engine::ParsedDependencyTree> RemoteResultsCollector::getDependencies()
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

    std::shared_ptr<STI::Engine::ParsedDependencyTree> tree;

    if (isDisabled()) {
        auto emptyTree = std::make_shared<STI::Engine::EventEngineDependencyTree>();
        tree = std::make_shared<STI::Engine::ParsedDependencyTree>(emptyTree);
        return tree; //empty
    }

    bool success = false;

	try {

		auto tTree = getTRef()->getDependencies();	//remote call

        if (tTree != 0) {
            success = convert<TEventEngineDependencyTree, std::shared_ptr<STI::Engine::ParsedDependencyTree>>(*tTree, tree);           
        }
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    if (!success) {
        auto emptyTree = std::make_shared<STI::Engine::EventEngineDependencyTree>();
        tree = std::make_shared<STI::Engine::ParsedDependencyTree>(emptyTree);
        return tree; //empty
    }

    return tree;
}

void RemoteResultsCollector::addEvents(const STI::Engine::DeviceEventMap& parsedEvents)
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

	if (isDisabled()) return;

    STI::TNetwork::TDeviceEventsSeq_var tEvents(new STI::TNetwork::TDeviceEventsSeq);

	try {

        convert<STI::Engine::DeviceEventMap, ::STI::TNetwork::TDeviceEventsSeq>(parsedEvents, tEvents);

		getTRef()->addEvents(tEvents);	//remote call

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteResultsCollector::addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files)
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

	if (isDisabled()) return;

    STI::TNetwork::TFileHolderSeq_var tFiles(new STI::TNetwork::TFileHolderSeq);

	try {

		convert<std::vector<std::shared_ptr<STI::Utils::FileHolder>>, STI::TNetwork::TFileHolderSeq>(files, tFiles);

		getTRef()->addTimingFiles(tFiles);	//remote call

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteResultsCollector::addMeasurements(const std::shared_ptr<STI::Engine::MeasurementVector>& measurements)
{
	std::unique_lock<std::mutex> collectorLock(collectorMutex);

	if (isDisabled() || measurements == 0) return false;
    
    bool success = false;

    STI::TNetwork::TMeasurementSeq_var tMeasurements(new STI::TNetwork::TMeasurementSeq);

	try {

		convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*measurements, tMeasurements);

		success = getTRef()->addMeasurements(tMeasurements);	//remote call

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
//bool RemoteResultsCollector::addAttributes(const STI::Device::DeviceID& deviceID, const std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes)
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

