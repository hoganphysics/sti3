
#include "ParseTicket.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_ParseTicket(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::ParseTicket>(m, "ParseTicket")

        //.def("getID", &STI::Network::HubID::id)
        
        ;

}
