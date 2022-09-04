
#include "PersistenceManagerPy.h"

#include <sti/engine/Measurement.h>
#include "MixedValuePy.h"
#include <sti/engine/ShotResult.h>
#include <sti/engine/RawEvent.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>


namespace py = pybind11;

using STI::Python::PersistenceManagerPy;
using STI::Python::MixedValuePy;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::RecordStatus;


void init_PersistenceManager(py::module& m)
{

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
        .def("getMeasurementGraphPath", &STI::Engine::Measurement::getMeasurementGraphPath)
        .def("dataReady", &STI::Engine::Measurement::dataReady)
        .def("print", &STI::Engine::Measurement::print)
        // .def("setMeasurementResult", py::overload_cast<STI::Engine::Measurement&, const MixedValuePy&>(
        //     [](STI::Engine::Measurement& self, const MixedValuePy& result)->void {
        //         self.setMeasurementResult(result);
        //     }), py::arg("result"))
        .def("setMeasurementResult", 
            [](STI::Engine::Measurement& self, const MixedValuePy& result)->void {
                self.setMeasurementResult(result);
            }, py::arg("result"))
        .def("setMeasurementResult", 
            [](STI::Engine::Measurement& self, const py::object& obj)->void {
                MixedValuePy result(obj);
                self.setMeasurementResult(result);
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


    py::enum_<RecordStatus>(m, "RecordStatus")      //{ Unqueried, Complete, MissingDevice, MissingResults, Error }
        .value("Unqueried", RecordStatus::Unqueried)
        .value("Complete", RecordStatus::Complete)
        .value("MissingDevice", RecordStatus::MissingDevice)
        .value("MissingResults", RecordStatus::MissingResults)
        .value("Error", RecordStatus::Error)
        .export_values()
        ;

    py::class_<ShotResultRecord>(m, "ShotResultRecord")
        .def(py::init<>())
        // .def("year", &ShotResult::year)
        .def_readonly("deviceID", &ShotResultRecord::deviceID)
        .def_readonly("recordStatus", &ShotResultRecord::recordStatus)
        .def_readonly("dependencies", &ShotResultRecord::dependencies)
        
        .def("__repr__",
            [](const ShotResultRecord& self) {
                return self.deviceID.getID() + " -> \n";
            })
        ;

    //ShotResult
    py::class_<ShotResult>(m, "ShotResult")
        .def(py::init<>())
        // .def("year", &ShotResult::year)
        .def_readonly("sid", &ShotResult::sid)
        .def_readonly("playTime", &ShotResult::playTime)
        // .def_readonly("parsedEvents", &ShotResult::parsedEvents)
        // .def_readonly("timingFiles", &ShotResult::timingFiles)
        .def("measurements", 
            [](const ShotResult& self) {
                if (self.measurements != 0) {
                    return *(self.measurements);
                }
                STI::Engine::MeasurementVector missing;
                return missing;
            })
        // .def_readonly("measurements", &ShotResult::measurements)
        // .def("measurements", 
        //     [](const ShotResult& self) {
        //         return self.measurements;
        //     })
        .def_readonly("attributes", &ShotResult::attributes)
        .def_readonly("shotResultRecord", &ShotResult::shotResultRecord)
        
        .def("__repr__",
            [](const ShotResult& self) {
                return self.sid.print();
            })
        .def("__eq__",  // operator ==
            [](const ShotResult& self, const ShotResult& other) {
                return self.sid == other.sid;
            })
        .def("__lt__",  // operator <
            [](const ShotResult& self, const ShotResult& other) {
                return self.sid < other.sid;
            })
        ;



    py::class_<PersistenceManagerPy, std::shared_ptr<PersistenceManagerPy>>(m, "PersistenceManager")
        .def("getShot", &PersistenceManagerPy::getShot, py::arg("shotID"))
        .def("getMeasurements", &PersistenceManagerPy::getMeasurements, py::arg("shotID"))
        ;

}
