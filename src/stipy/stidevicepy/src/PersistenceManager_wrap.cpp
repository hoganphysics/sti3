
#include "PersistenceManagerPy.h"

#include <sti/engine/Measurement.h>
#include "MixedValuePy.h"
#include <sti/engine/ShotResult.h>
#include <sti/engine/RawEvent.h>

#include <sti/engine/ParseResult.h>
#include <sti/engine/SequenceResult.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>


// PYBIND11_MAKE_OPAQUE(std::vector<std::shared_ptr<STI::Engine::Measurement>>);



namespace py = pybind11;

using STI::Python::PersistenceManagerPy;
using STI::Python::MixedValuePy;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::RecordStatus;


void init_PersistenceManager(py::module& m)
{
    py::class_<PersistenceManagerPy, std::shared_ptr<PersistenceManagerPy>>(m, "PersistenceManager")
        .def("findShot", &PersistenceManagerPy::findShot, py::arg("shotID"))
        .def("getParseResult", &PersistenceManagerPy::getParseResult, py::arg("parseID"))
        .def("getShotResult", &PersistenceManagerPy::getShotResult, py::arg("shotID"))
        .def("getSequenceResult", &PersistenceManagerPy::getSequenceResult, py::arg("sequenceID"))
        .def("getMeasurements", &PersistenceManagerPy::getMeasurements, py::arg("shotID"))
        .def("getFileServer", &PersistenceManagerPy::getFileServer)
        ;

}
