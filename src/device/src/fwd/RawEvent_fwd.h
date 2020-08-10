#ifndef STI_ENGINE_RAWEVENT_FWD_H
#define STI_ENGINE_RAWEVENT_FWD_H

#include "fwd/DeviceID_fwd.h"

#include <vector>
#include <map>
#include <memory>

namespace STI
{
namespace Engine
{

class RawEvent;
typedef std::vector<RawEvent> RawEventVector;
typedef std::map<double, RawEventVector> RawEventMap;

typedef std::map<STI::Device::DeviceID, RawEventVector> DeviceEventMap;
typedef std::shared_ptr<DeviceEventMap> DeviceEventMap_ptr;

} //Engine
} //STI

#endif
