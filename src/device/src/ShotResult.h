
#ifndef STI_ENGINE_SHOTRESULT_H
#define STI_ENGINE_SHOTRESULT_H

#include "DeviceID.h"
#include "fwd/Measurement_fwd.h"
#include "fwd/RawEvent_fwd.h"
#include "ShotID.h"
#include "utils/FileHolder.h"
#include "Attribute.h"

#include <vector>
#include <memory>
#include <map>
#include <string>

namespace STI
{
namespace Engine
{


class ShotResult
{
public:

    ShotID sid;

    DeviceEventMap parsedEvents;
    std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
    std::shared_ptr<MeasurementVector> measurements;
    //std::vector<std::shared_ptr<STI::Device::Attribute>> attributes;
    std::map<STI::Device::DeviceID, std::map<std::string, std::string>> attributes;

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

