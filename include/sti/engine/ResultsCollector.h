#ifndef STI_ENGINE_RESULTSCOLLECTOR_H
#define STI_ENGINE_RESULTSCOLLECTOR_H

#include <sti/engine/ShotID.h>
#include <sti/fwd/Measurement_fwd.h>
#include <sti/utils/FileServer.h>
#include <sti/device/Attribute.h>
#include <sti/device/VersionInfo.h>
#include <sti/fwd/RawEvent_fwd.h>

#include <map>
#include <memory>
#include <vector>


namespace STI
{
namespace Engine
{

class ResultsTicket;
class ParsedDependencyTree;
class ResultsCollectorFactory;
class EnginePlayingMessage;


class ResultsCollector
{
public:

    virtual ~ResultsCollector() {}

    virtual ShotID getShotID() const = 0;

    virtual bool addMeasurements(const STI::Device::DeviceID& deviceID, const MeasurementVector& measurements, const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer) = 0;
    virtual bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes) = 0;
    virtual bool addVersionInfo(const STI::Device::DeviceID& deviceID, const std::vector<STI::Device::VersionInfo>& versions) = 0;
    virtual bool addMessages(const std::vector<EnginePlayingMessage>& messages) = 0;
};


} //Engine
} //STI

#endif
