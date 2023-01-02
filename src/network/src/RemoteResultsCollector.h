#ifndef STI_NETWORK_REMOTERESULTSCOLLECTOR_H
#define STI_NETWORK_REMOTERESULTSCOLLECTOR_H

#include "deviceNet.h"
#include <sti/engine/ResultsCollector.h>
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

    bool addMeasurements(const STI::Device::DeviceID& deviceID, const std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);

private:

	mutable std::mutex collectorMutex;

};


} //Network
} //STI


#endif

