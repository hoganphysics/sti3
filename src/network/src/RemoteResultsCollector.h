#ifndef STI_NETWORK_REMOTERESULTSCOLLECTOR_H
#define STI_NETWORK_REMOTERESULTSCOLLECTOR_H

#include "generated/deviceNet.h"
#include <sti/engine/ResultsCollector.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteResultsCollector : public STI::Engine::ResultsCollector,
							   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TResultsCollector>	//mixin
{
public:

	RemoteResultsCollector(::STI::TNetwork::TResultsCollector_var collector);
    ~RemoteResultsCollector();

    STI::Engine::ShotID getShotID() const;

    bool addMeasurements(const STI::Device::DeviceID& deviceID, const STI::Engine::MeasurementVector& measurements, const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);
    bool addVersionInfo(const STI::Device::DeviceID& deviceID, const std::vector<STI::Device::VersionInfo>& versions);
    bool addMessages(const std::vector<STI::Engine::EnginePlayingMessage>& messages);

private:

	mutable std::mutex collectorMutex;

};


} //Network
} //STI


#endif
