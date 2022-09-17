
#include "SynchronousEventPy.h"
#include "SynchronousEventPyManager.h"

#include <memory>

using STI::Python::SynchronousEventPy;
using STI::Python::SynchronousEventPyManager;


std::shared_ptr<SynchronousEventPyManager> SynchronousEventPy::pyEventManager = 0;  


void SynchronousEventPy::addPyReference(const std::shared_ptr<STI::Engine::SynchronousEvent>& value)
{
    auto sepy = std::dynamic_pointer_cast<SynchronousEventPy>(value);

    if (sepy) {
        pybind11::object obj = pybind11::cast(value);

        if (pyEventManager != 0) {
            pyEventManager->addPyEvent(obj);
        }
    }
}

void SynchronousEventPy::clearPyRefs()
{
    if (pyEventManager != 0) {
        pyEventManager->clear();
    }
}

