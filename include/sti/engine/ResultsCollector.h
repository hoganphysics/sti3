#ifndef STI_ENGINE_RESULTSCOLLECTOR_H
#define STI_ENGINE_RESULTSCOLLECTOR_H

#include <sti/engine/ShotID.h>
#include <sti/fwd/Measurement_fwd.h>
#include <sti/utils/FileHolder.h>
#include <sti/device/Attribute.h>
#include <sti/fwd/RawEvent_fwd.h>

#include <memory>


namespace STI
{
namespace Engine
{

class ResultsTicket;
class ParsedDependencyTree;


class ResultsCollector
{
public:

    virtual ~ResultsCollector() {}

    virtual ShotID getShotID() const = 0;
    //virtual std::shared_ptr<ParsedDependencyTree> getDependencies() = 0;

    virtual void addEvents(const DeviceEventMap& parsedEvents) = 0;
    virtual void addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files) = 0;
    virtual bool addMeasurements(const std::shared_ptr<MeasurementVector>& measurements) = 0;
    virtual bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes) = 0;

};


} //Engine
} //STI

#endif
