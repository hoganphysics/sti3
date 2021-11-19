#ifndef STI_NETWORK_REMOTERESULTSCOLLECTOR_H
#define STI_NETWORK_REMOTERESULTSCOLLECTOR_H

#include "deviceNet.h"
#include "ResultsCollector.h"
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>


namespace STI
{
namespace Network
{

class RemoteResultsCollector : public STI::Engine::ResultsCollector,
							   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TResultsCollector>	//mixin
{
public:

	RemoteResultsCollector(::STI::TNetwork::TResultsCollector_ptr collector);
    ~RemoteResultsCollector();

    STI::Engine::ShotID getShotID() const;
    std::shared_ptr<STI::Engine::ParsedDependencyTree> getDependencies();

    void addEvents(const STI::Engine::DeviceEventMap& parsedEvents);
    void addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files);
    bool addMeasurements(const std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
    //bool addAttributes(const STI::Device::DeviceID& deviceID, const std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);

private:

	mutable std::mutex collectorMutex;

};


} //Network
} //STI


#endif

