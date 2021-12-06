
#include "SynchronousEventPyManager.h"


#include <algorithm>
#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::SynchronousEventPyManager;


SynchronousEventPyManager::SynchronousEventPyManager() 
{

}
SynchronousEventPyManager::~SynchronousEventPyManager()
{
    clear();
}

void SynchronousEventPyManager::addPyEvent(const pybind11::object& evt)
{
    pybind11::gil_scoped_acquire acquire;

    auto it = std::find_if(pySynchronousEventsRefs.begin(), pySynchronousEventsRefs.end(), 
        [&evt](const pybind11::object& obj)
        {
            return obj.is(evt);
        });
    
    if (it == pySynchronousEventsRefs.end()) {
        pySynchronousEventsRefs.push_back(evt);
    }
}

void SynchronousEventPyManager::clear()
{
    pybind11::gil_scoped_acquire acquire;
    pySynchronousEventsRefs.clear();
}

