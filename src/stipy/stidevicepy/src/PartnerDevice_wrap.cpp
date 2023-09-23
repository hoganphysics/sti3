#include <sti/device/PartnerDevice.h>
#include "PartnerDevicePy.h"

#include "MixedValuePy.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Python::PartnerDevicePy;
using STI::Engine::RawEvent;
using STI::Engine::RawEventTargetChannel;


void init_PartnerDevice(py::module& m) 
{

    py::class_<PartnerDevicePy, DevicePy, std::shared_ptr<PartnerDevicePy>>(m, "PartnerDevice") 

        .def("addEvent", 
            py::overload_cast<const RawEvent&, const RawEvent&>(&PartnerDevicePy::addEvent), 
            py::arg("evt"), py::arg("referenceEvent"))
        .def("addEvent", 
            py::overload_cast<double, const RawEventTargetChannel&, const pybind11::object&, const RawEvent&>
                (&PartnerDevicePy::addEvent), 
            py::arg("time"), py::arg("channel"), py::arg("value"), py::arg("referenceEvent"))
        .def("addEvent", 
            py::overload_cast<double, const RawEventTargetChannel&, const pybind11::object&, const STI::Engine::RawEventType&, const RawEvent&>
                (&PartnerDevicePy::addEvent), 
            py::arg("time"), py::arg("channel"), py::arg("value"), py::arg("eventType"), py::arg("referenceEvent"))

        ;

}

