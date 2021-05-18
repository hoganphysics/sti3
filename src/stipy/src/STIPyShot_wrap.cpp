
#include "STIPyShot.h"
#include "STIPyChannel.h"
#include "RawEvent.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;


void init_STIPyShot(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::STIPyShot, std::shared_ptr<STI::Python::STIPyShot>>(m, "STIPyShot")

        .def("event", &STI::Python::STIPyShot::event)
        .def("meas", &STI::Python::STIPyShot::meas)
        .def("getEvents", &STI::Python::STIPyShot::getEvents)
        // .def("getEvents",
        //     [](STI::Python::STIPyShot& self) {
        //         std::shared_ptr<std::vector<STI::Engine::RawEvent>> evts;
        //         self.getEvents(evts);
        //         return (*evts);
        //     }
        // )
        ;

}

