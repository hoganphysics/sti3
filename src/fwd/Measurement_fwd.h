#ifndef STI_ENGINE_MEASUREMENT_FWD_H
#define STI_ENGINE_MEASUREMENT_FWD_H

#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class Measurement;

typedef std::vector<std::shared_ptr<Measurement>> MeasurementVector;

} //Engine
} //STI

#endif
