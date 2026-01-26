#include <sti/engine/ShotResult.h>

#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotResultRecord.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>

#include <sstream>

using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::RecordStatus;

namespace py = pybind11;


void getMeasurements(const STI::Engine::MeasurementVector& input, STI::Engine::MeasurementVector& output, const STI::Device::DeviceID& deviceID)
{
    std::copy_if(input.begin(), input.end(), std::back_inserter(output), 
        [&deviceID](auto& m) { return (m != 0 && m->device() == deviceID); });
}

void init_ShotResult(py::module& m)
{
    //RecordStatus { Unqueried, Complete, MissingDevice, MissingResults, Error };
    py::enum_<RecordStatus>(m, "RecordStatus")
        .value("Unqueried", RecordStatus::Unqueried)
        .value("Complete", RecordStatus::Complete)
        .value("MissingDevice", RecordStatus::MissingDevice)
        .value("MissingResults", RecordStatus::MissingResults)
        .value("Error", RecordStatus::Error)
        ;
        //.export_values();


    py::class_<ShotResultRecord>(m, "ShotResultRecord")
        .def("isPartialRecord", &ShotResultRecord::isPartialRecord)
        .def_readonly("deviceID", &ShotResultRecord::deviceID)
        .def_readonly("recordStatus", &ShotResultRecord::recordStatus)
        .def_readonly("dependencies", &ShotResultRecord::dependencies)
                .def("__repr__",
            [](const ShotResultRecord& self) {
                std::stringstream s;
                s << "<ShotResultRecord (" << self.deviceID.getID()
                << ") | " << ShotResultRecord::statusToString(self.recordStatus) 
                  << " | Dependencies: " << self.dependencies.size() << ">";
                return s.str();
            })
        ;


    py::class_<ShotResult, std::shared_ptr<ShotResult>>(m, "ShotResult")
        .def_readonly("sid", &ShotResult::sid)
        .def_readonly("playTime", &ShotResult::playTime)
        // .def_readonly("attributes", &ShotResult::attributes)
        .def_readonly("messages", &ShotResult::messages)
        .def("getAttributes",
            [](ShotResult& self) {
                py::dict attrs;
                for (auto& dev : self.attributes) {
                    py::dict devAttrs;
                    for (auto& attr : dev.second) {
                        devAttrs[py::str(attr.first)] = py::str(attr.second);
                    }
                    attrs[py::cast(dev.first)] = devAttrs;
                }
                return attrs;
            })
        .def("getMeasurements",
            [](ShotResult& self) -> STI::Engine::MeasurementMap {
                if (self.measurements != 0) {
                    return *self.measurements;
                }
                STI::Engine::MeasurementMap missing;
                return missing;
            })

        // .def_readonly("measurements", &ShotResult::measurements)

        // .def("getMeasurements",
        //     [](ShotResult& self) -> STI::Engine::MeasurementVector {
        //         if (self.measurements != 0) {
        //             return *self.measurements;
        //         }
        //         STI::Engine::MeasurementVector missing;
        //         return missing;
        //     })
        // .def("getMeasurements",
        //     [](ShotResult& self, const STI::Device::DeviceID& deviceID) -> STI::Engine::MeasurementVector {
        //         STI::Engine::MeasurementVector selected;
        //         if (self.measurements != 0) {
        //             getMeasurements(*self.measurements, selected, deviceID);
        //         }
        //         return selected;
        //     }, py::arg("deviceID"))
        // .def("getMeasurements",
        //     [](ShotResult& self, const std::string& deviceID) -> STI::Engine::MeasurementVector {
        //         STI::Device::DeviceID id(deviceID);
                
        //         STI::Engine::MeasurementVector selected;
        //         if (self.measurements != 0) {
        //             getMeasurements(*self.measurements, selected, id);
        //         }
        //         return selected;
        //     }, py::arg("deviceID"))
        .def_readonly("shotResultRecord", &ShotResult::shotResultRecord)
        .def("__eq__",  // operator ==
            [](const ShotResult& self, const ShotResult& other) {
                return self.sid == other.sid;
            })
        .def("__lt__",  // operator <
            [](const ShotResult& self, const ShotResult& other) {
                return self.sid < other.sid;
            })
        .def("__repr__",
            [](const ShotResult& self) {
                std::stringstream s;
                // <ParseResult | pid:user@machine#2022_05_12>
                s << "<ShotResult | " << self.sid.print() << ">";
                return s.str();
            })
        ;

}
