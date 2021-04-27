
#include "ResultTicket.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_ResultTicket(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::ResultTicket>(m, "ResultTicket")

        //.def("getID", &STI::Network::HubID::id)
        
        ;

}
