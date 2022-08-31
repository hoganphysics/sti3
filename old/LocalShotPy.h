
#ifndef STI_PYTHON_LOCALSHOTPY_H
#define STI_PYTHON_LOCALSHOTPY_H

#include "LocalShot.h"
#include <sti/fwd/RawEvent_fwd.h>


#include <vector>
#include <memory>

//#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

class LocalShotPy : public STI::Engine::LocalShot
{
public:

    LocalShotPy(const STI::Engine::ShotConfig& config);
    ~LocalShotPy();

    std::vector<STI::Engine::RawEvent> getEvents();

};


} //Python
} //STI

#endif

