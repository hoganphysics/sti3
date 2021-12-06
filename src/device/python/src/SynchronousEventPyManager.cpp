
#include "SynchronousEventPyManager.h"

#include <iostream>

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



    // //Very important to hold the GIL during python object destruction below
    // pybind11::gil_scoped_acquire acquire;

    // std::cout << "~SynchronousEventPyManager() " << pySynchronousEventsRefs.size() << std::endl;

    // for (auto& ref : pySynchronousEventsRefs) {
    //     std::cout << "~SynchronousEventPyManager() ref=" << ref.ref_count() << std::endl;
    //     // ref.inc_ref();
    //     // ref.release();
    //     // ref.dec_ref();
    //     // ref.inc_ref();
    //     // ref.dec_ref();
    //     // auto r = ref.release();
    //     std::cout << "----> ref=" << ref.ref_count() << std::endl;
    // }


    // try {
    //     pySynchronousEventsRefs.clear();
    // }
    // catch (py::error_already_set& e) {
    //         std::cout << "SynchronousEventPyManager exception: " << e.what() << std::endl;
    // }
        
    //     std::cout << "~SynchronousEventPyManager() cleared " << pySynchronousEventsRefs.size() << std::endl;
    // }
}

void SynchronousEventPyManager::addPyEvent(const pybind11::object& evt)
{
    pybind11::gil_scoped_acquire acquire;
    std::cout << "%%%%%%%%%% SynchronousEventPyManager()::addPyEvent ref=" << evt.ref_count() << std::endl;
    // evt.inc_ref();

    auto it = std::find_if(pySynchronousEventsRefs.begin(), pySynchronousEventsRefs.end(), 
        [&evt](const pybind11::object& obj)
        {
            return obj.is(evt);
        });
    
    if (it == pySynchronousEventsRefs.end()) {
        std::cout << "$$$$$$  object not found" << std::endl;
        pySynchronousEventsRefs.push_back(evt);
    }
    else {
        std::cout << "$$$$$$  Found object" << std::endl;
        // pySynchronousEventsRefs.push_back(evt);
    }

}

void SynchronousEventPyManager::clear()
{
    pybind11::gil_scoped_acquire acquire;
    pySynchronousEventsRefs.clear();
}

