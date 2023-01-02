#ifndef STI_ENGINE_MEASUREMENT_FWD_H
#define STI_ENGINE_MEASUREMENT_FWD_H

#include <sti/fwd/DeviceID_fwd.h>

#include <vector>
#include <map>
#include <memory>

namespace STI
{
namespace Engine
{

class Measurement;

typedef std::vector<std::shared_ptr<Measurement>> MeasurementVector;
typedef std::map<STI::Device::DeviceID, std::shared_ptr<MeasurementVector>> MeasurementMap;

} //Engine
} //STI

#endif
