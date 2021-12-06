#ifndef STI_ENGINE_SYNCHRONOUSEVENT_FWD_H
#define STI_ENGINE_SYNCHRONOUSEVENT_FWD_H

#include <vector>
#include <memory>

namespace STI
{
namespace Engine
{

class SynchronousEvent;
// typedef std::vector<std::unique_ptr<SynchronousEvent>> SynchronousEventVector;
typedef std::vector<std::shared_ptr<SynchronousEvent>> SynchronousEventVector;

} //Engine
} //STI

#endif
