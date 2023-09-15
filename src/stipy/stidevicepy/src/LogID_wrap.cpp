
#include <sti/device/LogID.h>

#include <sstream>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Device::LogID;


void init_LogID(py::module& m) 
{

    py::class_<LogID>(m, "LogID")
        .def(py::init<>())
        .def(py::init<const STI::Device::DeviceID&, const std::string&, const std::string&, unsigned>(), 
                        py::arg("deviceID"), py::arg("date"), py::arg("logName"), py::arg("index") )
        .def_readwrite("deviceID", &LogID::deviceID)
        .def_readwrite("date", &LogID::date)
        .def_readwrite("logName", &LogID::logName)
        .def_readwrite("index", &LogID::index)
        .def("__repr__",
            [](const LogID& id) {
                std::stringstream s;
                s << "<LogID | " 
                  << id.deviceID.getID() << " | " 
                  << id.date << " | " 
                  << id.logName << " | " 
                  << id.index << ">";
                return s.str();
            })
        .def("__eq__",  // operator ==
            [](const LogID& self, const LogID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const LogID& self, const LogID& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const LogID& self, const LogID& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;


}

