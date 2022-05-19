
#include "SynchronousEventPy.h"

#include <memory>

using STI::Python::SynchronousEventPy;
using STI::Python::SynchronousEventPyManager;


// std::shared_ptr<SynchronousEventPyManager> SynchronousEventPy::pyEventManager = std::make_shared<SynchronousEventPyManager>();  
std::shared_ptr<SynchronousEventPyManager> SynchronousEventPy::pyEventManager = 0;  


