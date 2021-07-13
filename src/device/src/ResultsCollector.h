#ifndef STI_ENGINE_RESULTSCOLLECTOR_H
#define STI_ENGINE_RESULTSCOLLECTOR_H

#include "ShotID.h"
#include "fwd/Measurement_fwd.h"
#include "utils/FileHolder.h"
#include "Attribute.h"
#include "fwd/RawEvent_fwd.h"

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

    virtual ShotID getShotID() = 0;
    virtual std::shared_ptr<ParsedDependencyTree> getDependencies() = 0;

    virtual void addEvents(const DeviceEventMap& parsedEvents) = 0;
    virtual void addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files) = 0;
    virtual bool addMeasurements(const std::shared_ptr<MeasurementVector>& measurements) = 0;
    virtual bool addAttributes(const STI::Device::DeviceID& deviceID, const std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes) = 0;

};


} //Engine
} //STI

#endif
