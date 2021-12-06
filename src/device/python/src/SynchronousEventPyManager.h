
#ifndef STI_PYTHON_SYNCHRONOUSEVENTPYMANAGER_H
#define STI_PYTHON_SYNCHRONOUSEVENTPYMANAGER_H


#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{


class SynchronousEventPyManager
{
public:

    SynchronousEventPyManager();
    virtual ~SynchronousEventPyManager();

    void addPyEvent(const pybind11::object& evt);
    void clear();

private:

    std::vector<pybind11::object> pySynchronousEventsRefs;
};


} //Python
} //STI

#endif

