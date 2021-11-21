
#include "SynchronousEventPy.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>


namespace py = pybind11;

using STI::Python::SynchronousEventPy;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventAdapter;

void init_SynchronousEvent(py::module& m)
{
    py::class_<SynchronousEvent, std::shared_ptr<SynchronousEvent>>(m, "SynchronousEventBase")
        ;

    py::class_<SynchronousEventAdapter, SynchronousEvent, SynchronousEventPy, std::shared_ptr<SynchronousEventAdapter>>(m, "SynchronousEvent")
        .def(py::init<double>(), py::arg("time"))
        .def("loadEvent", &SynchronousEventAdapter::loadEvent)
        .def("playEvent", &SynchronousEventAdapter::playEvent)
        .def("collectMeasurementData", &SynchronousEventAdapter::collectMeasurementData)
        .def("stopEvent", &SynchronousEventAdapter::stopEvent)
        .def("pauseEvent", &SynchronousEventAdapter::pauseEvent)
        .def("unpauseEvent", &SynchronousEventAdapter::unpauseEvent, py::arg("retrigger"))
        .def("waitBeforePlay", &SynchronousEventAdapter::waitBeforePlay)
        .def("waitBeforeCollectData", &SynchronousEventAdapter::waitBeforeCollectData)
        ;

}
