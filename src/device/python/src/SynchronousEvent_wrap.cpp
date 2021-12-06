
#include "SynchronousEventPy.h"
#include "fwd/SynchronousEvent_fwd.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>

#include <pybind11/stl_bind.h>

#include "stl_bind_pyref.h"

#include <iostream>

// PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::SynchronousEvent>>);

namespace py = pybind11;

using STI::Python::SynchronousEventPy;
using STI::Engine::SynchronousEvent;
using STI::Engine::SynchronousEventAdapter;

void init_SynchronousEvent(py::module& m)
{

    // py::class_<STI::Engine::SynchronousEventVector>(m, "SynchronousEventVector")
    //     .def("append2", [](STI::Engine::SynchronousEventVector& v, const std::shared_ptr<STI::Engine::SynchronousEvent>& value)
    //         {
    //             SynchronousEventPy::addPyReference(value);
    //             v.push_back(std::move(value));
    //         })
    //     ;

    // auto vecCl = py::bind_vector<std::vector<std::shared_ptr<STI::Engine::SynchronousEvent>>, std::shared_ptr<std::vector<std::shared_ptr<STI::Engine::SynchronousEvent>>>>(m, "SynchronousEventVector");
    
    auto vecCl = STI::Python::bind_vector_pyref<STI::Engine::SynchronousEventVector, SynchronousEventPy>(m, "SynchronousEventVector");

    // auto vecCl = STI::Python::bind_vector_pyref<STI::Engine::SynchronousEventVector, std::unique_ptr<STI::Engine::SynchronousEventVector, py::nodelete>>(m, "SynchronousEventVector");

    vecCl.def("append2",
            [](STI::Engine::SynchronousEventVector& v, const std::shared_ptr<STI::Engine::SynchronousEvent>& value) 
            {
                

                // auto sepy = std::dynamic_pointer_cast<SynchronousEventPy>(value);

                // if (sepy) {

                //     // sepy->
                //     // sepy->obj = py::cast(value);
                //     // sepy->obj.inc_ref();

                //     // py::object obj = py::cast(value);
                //     // obj.inc_ref();

                //     std::cout << ".append2 DYANMIC success " << value.use_count() << std::endl;
                // }
                // else {
                //     std::cout << ".append2 DYANMIC failed " << value.use_count() << std::endl;
                // }

                // std::cout << ".append count=" << value.use_count() << std::endl;
                v.push_back(value);

                SynchronousEventPy::addPyReference(value);

                // std::cout << "after push .append count=" << value.use_count() << std::endl;
            },
            py::arg("x"),
            "Add an item to the end of the list"); //py::keep_alive<1, 2>() //py::return_value_policy::reference_internal

    // py::class_<SynchronousEvent, std::shared_ptr<SynchronousEvent>>(m, "SynchronousEventBase")
    //     ;

    // py::class_<SynchronousEventAdapter, SynchronousEvent, SynchronousEventPy, std::shared_ptr<SynchronousEventAdapter>>(m, "SynchronousEventAdapter")
    //     .def(py::init<double>(), py::arg("time"))
    //     .def("loadEvent", &SynchronousEventAdapter::loadEvent)
    //     .def("play", &SynchronousEventAdapter::play)
    //     .def("playEvent", &SynchronousEventAdapter::playEvent)
    //     .def("collectMeasurementData", &SynchronousEventAdapter::collectMeasurementData)
    //     .def("stopEvent", &SynchronousEventAdapter::stopEvent)
    //     .def("pauseEvent", &SynchronousEventAdapter::pauseEvent)
    //     .def("unpauseEvent", &SynchronousEventAdapter::unpauseEvent, py::arg("retrigger"))
    //     .def("waitBeforePlay", &SynchronousEventAdapter::waitBeforePlay)
    //     .def("waitBeforeCollectData", &SynchronousEventAdapter::waitBeforeCollectData)
    //     ;



    py::class_<SynchronousEvent, std::shared_ptr<SynchronousEvent>, SynchronousEventPy>(m, "SynchronousEvent")  //py::nodelete
        .def(py::init<double>(), py::arg("time"))
        // .def("add", [](const std::shared_ptr<STI::Engine::SynchronousEvent>& self)
        // .def("add", [](const std::shared_ptr<STI::Engine::SynchronousEvent>& self)
        // {
        //     auto sepy = std::dynamic_pointer_cast<SynchronousEventPy>(self);
        //     if (sepy) {
        //         sepy->obj = py::cast(self);
        //     }
        // })
        // .def("add2", [](STI::Engine::SynchronousEvent& self)
        // {
        //     SynchronousEventPy& selfpy = dynamic_cast<SynchronousEventPy&>(self);
        //     selfpy.obj = py::cast(self);
        // })

        .def("loadEvent", &SynchronousEvent::loadEvent)
        .def("play", &SynchronousEvent::play)
        .def("playEvent", &STI::Engine::SynchronousEvent::playEvent)
        .def("collectMeasurementData", &SynchronousEvent::collectMeasurementData)
        .def("stopEvent", &SynchronousEvent::stopEvent)
        .def("pauseEvent", &SynchronousEvent::pauseEvent)
        .def("unpauseEvent", &SynchronousEvent::unpauseEvent, py::arg("retrigger"))
        .def("waitBeforePlay", &SynchronousEvent::waitBeforePlay)
        .def("waitBeforeCollectData", &SynchronousEvent::waitBeforeCollectData)
        ;



}
