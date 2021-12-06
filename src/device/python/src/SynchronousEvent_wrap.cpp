
#include "SynchronousEventPy.h"
#include "fwd/SynchronousEvent_fwd.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

#include "stl_bind_pyref.h"


using STI::Python::SynchronousEventPy;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventAdapter;

void init_SynchronousEvent(py::module& m)
{

    auto vecCl = STI::Python::bind_vector_pyref<STI::Engine::SynchronousEventVector, SynchronousEventPy>(m, "SynchronousEventVector");


    py::class_<SynchronousEvent, std::shared_ptr<SynchronousEvent>>(m, "SynchronousEventBase")
        ;

    py::class_<SynchronousEventAdapter, std::shared_ptr<SynchronousEventAdapter>, SynchronousEventPy, SynchronousEvent>(m, "SynchronousEvent")  //py::nodelete
        .def(py::init<double>(), py::arg("time"))
        .def("loadEvent", &SynchronousEventAdapter::loadEvent)
        .def("play", &SynchronousEventAdapter::play)
        .def("playEvent", &STI::Engine::SynchronousEventAdapter::playEvent)
        .def("collectMeasurementData", &SynchronousEventAdapter::collectMeasurementData)
        .def("stopEvent", &SynchronousEventAdapter::stopEvent)
        .def("pauseEvent", &SynchronousEventAdapter::pauseEvent)
        .def("unpauseEvent", &SynchronousEventAdapter::unpauseEvent, py::arg("retrigger"))
        .def("waitBeforePlay", &SynchronousEventAdapter::waitBeforePlay)
        .def("waitBeforeCollectData", &SynchronousEventAdapter::waitBeforeCollectData)
        ;

}
