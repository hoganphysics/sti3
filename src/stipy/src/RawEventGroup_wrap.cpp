
#include "RawEventGroup.h"

#include <sti/utils/utils.h>

#include <sstream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Engine::RawEventGroup;


void init_RawEventGroup(py::module& m)
{

    py::class_<RawEventGroup>(m, "RawEventGroup")
        .def(py::init<>())
        .def(py::init<const std::string&, const STI::Utils::GraphPathLabel&>(), 
            py::arg("groupName"), py::arg("groupIndex"))
        .def("getName", py::overload_cast<>(&RawEventGroup::getName, py::const_))
        
        .def("startTime", py::overload_cast<>(&RawEventGroup::startTime, py::const_))
        .def("endTime", py::overload_cast<>(&RawEventGroup::endTime, py::const_))
        .def("setStartTime", py::overload_cast<double>(&RawEventGroup::setStartTime), py::arg("startTime"))
        .def("setEndTime", py::overload_cast<double>(&RawEventGroup::setEndTime), py::arg("endTime"))
        
        .def("getIndex", py::overload_cast<>(&RawEventGroup::getIndex, py::const_))
        .def("getFullIndex", py::overload_cast<>(&RawEventGroup::getFullIndex, py::const_))
        // .def("getParentGroupIndex", py::overload_cast<>(&RawEventGroup::getParentGroupIndex, py::const_))
        // .def("getTargetServerID", py::overload_cast<>(&RawEventGroup::getTargetServerID, py::const_))
        
        .def("__repr__",
            [](const RawEventGroup& self) {
                std::stringstream s;
                s << "group('" << self.getName() 
                << "', index=" << self.getIndex()
                << ", time=[" << STI::Utils::printTimeFormated(self.startTime())
                << ", " << STI::Utils::printTimeFormated(self.endTime())
                << "])";
                return s.str();
            })
        ;

}
