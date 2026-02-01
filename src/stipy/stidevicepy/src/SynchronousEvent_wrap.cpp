
#include "SynchronousEventPy.h"
#include <sti/fwd/SynchronousEvent_fwd.h>
#include <sti/utils/utils.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/Measurement.h>

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "stl_bind_pyref.h"


using STI::Python::SynchronousEventPy;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventAdapter;

void init_SynchronousEvent(py::module& m)
{

    STI::Python::bind_vector_pyref<STI::Engine::SynchronousEventVector, SynchronousEventPy>(m, "SynchronousEventVector");


    py::class_<SynchronousEvent, std::shared_ptr<SynchronousEvent>>(m, "SynchronousEventBase")
        ;

    py::class_<SynchronousEventAdapter, std::shared_ptr<SynchronousEventAdapter>, SynchronousEventPy, SynchronousEvent>(m, "SynchronousEvent")  //py::nodelete
        .def(py::init<double>(), py::arg("time"))
        .def("getTime", &SynchronousEventAdapter::getTime)
        .def("getMeasurements", &SynchronousEventAdapter::getMeasurements)
        .def("addMeasurement", &SynchronousEventAdapter::addMeasurement)    //py::call_guard<py::gil_scoped_release>()
        .def("addError", &SynchronousEventAdapter::addError, py::return_value_policy::reference_internal)
        .def("addWarning", &SynchronousEventAdapter::addWarning, py::return_value_policy::reference_internal)
        .def("addInfoMessage", &SynchronousEventAdapter::addInfoMessage, py::return_value_policy::reference_internal)
        .def("getMessages", &SynchronousEventAdapter::getMessages)

        .def("loadEvent", &SynchronousEventAdapter::loadEvent)
        .def("play", &SynchronousEventAdapter::play)
        .def("playEvent", &STI::Engine::SynchronousEventAdapter::playEvent)
        .def("collectMeasurementData", &SynchronousEventAdapter::collectMeasurementData)
        .def("stopEvent", &SynchronousEventAdapter::stopEvent)
        .def("pauseEvent", &SynchronousEventAdapter::pauseEvent)
        .def("unpauseEvent", &SynchronousEventAdapter::unpauseEvent, py::arg("retrigger"))
        .def("waitBeforePlay", &SynchronousEventAdapter::waitBeforePlay)
        .def("waitBeforeCollectData", &SynchronousEventAdapter::waitBeforeCollectData)
        .def("__repr__",
            [](const SynchronousEventAdapter& self) {
                return "<SynchronousEvent @ " + STI::Utils::printTimeFormated(self.getTime()) + ">";
            })
        .def("__lt__",  // operator <
            [](const SynchronousEventAdapter& self, const SynchronousEventAdapter& rhs) {
                return self < rhs;
            })
        ;

}
