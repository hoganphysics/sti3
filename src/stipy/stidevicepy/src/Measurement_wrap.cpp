#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>

#include "MixedValuePy.h"

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

// PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);

using STI::Python::MixedValuePy;


void init_Measurement(py::module& m)
{

    // py::bind_vector<STI::Engine::MeasurementVector>(m, "MeasurementVector");

    //may need to manually wrap Measurement:
    //class MeasurementWrap: 
    //    MeasurementWrap(const std::shared_ptr<Measurement>)
    //Then use py::class_<STI::Engine::MeasurementWrap> and vector<MeasurementWrap>

    py::class_<STI::Engine::Measurement, std::shared_ptr<STI::Engine::Measurement>>(m, "Measurement")
        .def(py::init<>())
        .def("time", &STI::Engine::Measurement::time)
        .def("channel", &STI::Engine::Measurement::channel)
        .def("data",
            [](const STI::Engine::Measurement& self) {
                MixedValuePy pyval(self.data());
                return pyval.getValue_py();
            })
        .def("device", &STI::Engine::Measurement::device)
        .def("groupName", &STI::Engine::Measurement::groupName)
        .def("getMeasurementGraphPath", &STI::Engine::Measurement::getMeasurementGraphPath)
        .def("dataReady", &STI::Engine::Measurement::dataReady)
        .def("print", &STI::Engine::Measurement::print)
        // .def("setMeasurementResult", py::overload_cast<STI::Engine::Measurement&, const MixedValuePy&>(
        //     [](STI::Engine::Measurement& self, const MixedValuePy& result)->void {
        //         self.setMeasurementResult(result);
        //     }), py::arg("result"))
        .def("setMeasurementResult", 
            [](STI::Engine::Measurement& self, const MixedValuePy& result)->void {
                self.setMeasurementResult(std::move(result));
            }, py::arg("result"))
        .def("setMeasurementResult", 
            [](STI::Engine::Measurement& self, const py::object& obj)->void {
                MixedValuePy result(obj);
                self.setMeasurementResult(std::move(result));
            }, py::arg("result"))
        .def("__repr__",
            [](const STI::Engine::Measurement& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const STI::Engine::Measurement& self, const STI::Engine::Measurement& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const STI::Engine::Measurement& self, const STI::Engine::Measurement& other) {
                return self < other;
            })
        ;

}
