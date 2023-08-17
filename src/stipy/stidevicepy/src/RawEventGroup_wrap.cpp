#include <sti/engine/RawEventGroup.h>

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/ParsedTag.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTraceData.h>
#include <sti/utils/utils.h>

#include "MixedValuePy.h"
#include "RawStackTrace.h"

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using STI::Python::MixedValuePy;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventType;
using STI::Engine::RawStackTrace;


void init_RawEventGroup(py::module& m)
{

    py::class_<RawEventGroup, std::shared_ptr<RawEventGroup>>(m, "RawEventGroup")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>(), py::arg("name"), py::arg("parentName"))
        
        .def("getName", py::overload_cast<>(&RawEventGroup::getName, py::const_))
        .def("getFullName", py::overload_cast<>(&RawEventGroup::getFullName, py::const_))
        .def("setName", &RawEventGroup::setName)

        .def("startTime", py::overload_cast<>(&RawEventGroup::startTime, py::const_))
        .def("endTime", py::overload_cast<>(&RawEventGroup::endTime, py::const_))

        .def("getTimeOffset", py::overload_cast<>(&RawEventGroup::getTimeOffset, py::const_))
        .def("shiftStartTimeTo", &RawEventGroup::shiftStartTimeTo, py::arg("time"))
        .def("shiftEndTimeTo", &RawEventGroup::shiftEndTimeTo, py::arg("time"))
        .def("shiftReferenceTimeTo", &RawEventGroup::shiftReferenceTimeTo, py::arg("refName"), py::arg("time"))

        .def("addReferencePoint", &RawEventGroup::addReferencePoint, py::arg("refName"), py::arg("time"))
        .def("getReferencePoint", 
            [](const RawEventGroup& self, const std::string& refName) {
                double time = 0;
                self.getReferencePoint(refName, time);
                return time;
            }, py::arg("refName"))
        .def("getReferencePoint",
            [](const RawEventGroup& self, const std::string& refName) {
                double time = 0;
                self.getReferencePoint(refName, time);
                return time;
            })
        .def("addMetadata",
            [](RawEventGroup& self, const std::string& key, const pybind11::object& data) {
                MixedValuePy mixedValue;
                mixedValue.setValue_py(data);
                self.addMetaData(key, mixedValue);
            })
        .def("metadata",
            [](RawEventGroup& self, const std::string& key) {
                MixedValuePy val = self.getMetaData(key);
                return val.getValue_py();                
            })

        .def("var", &RawEventGroup::var, py::arg("fullVarName"), py::arg("stackTrace"))

        .def("addvar", //name overriden to 'setvar' in python
            [](RawEventGroup& self, const std::string& fullVarName, 
                        const pybind11::object& value, const RawStackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.addvar(fullVarName, mixedValue, stackTrace);
            }, py::arg("fullVarName"), py::arg("value"), py::arg("stackTrace"))
        .def("addtag", //name overriden to 'settag' in python
            &RawEventGroup::addtag, py::arg("fullTagName"), py::arg("stackTrace"))
        .def("addEvent",    //name overriden to 'event' in python
            [](RawEventGroup& self, const STI::Engine::RawEventTarget& target, 
                double time, const pybind11::object& value,
                const STI::Engine::RawStackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
            })
        .def("addMeas", //name overriden to 'meas' in python
                [](RawEventGroup& self, const STI::Engine::RawEventTarget& target, 
                        double time, const pybind11::object& value,
                        const STI::Engine::RawStackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
            }, py::arg("target"), py::arg("time"), py::arg("value"), py::arg("stackTrace"))

        .def("bindvar", [](RawEventGroup& self, const std::string& fullVarName, 
                        const pybind11::object& value) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.bindVar(fullVarName, mixedValue);
            }, py::arg("fullVarName"), py::arg("value"))

        .def("group", &RawEventGroup::group, py::arg("name"))
        .def("subgroups", &RawEventGroup::getSubgroups)

        // .def("getEvents", &RawEventGroup::getEvents, py::const_)
        .def("events", 
            [](const RawEventGroup& self) {
                auto evts = self.getEvents();

                if (evts != 0) {
                    return *(evts);
                }

                STI::Engine::RawEventVector emptyEvents;
                return emptyEvents;
            }
        )
        // .def("getVars", &RawEventGroup::getVars, py::const_)
        // .def("getTags", &RawEventGroup::getTags, py::const_)

        .def("vars", &RawEventGroup::getVars)
        .def("tags", &RawEventGroup::getTags)
        .def("overwrittenVars", &RawEventGroup::getOverwrittenVars)
        
        .def("__repr__",
            [](const RawEventGroup& self) {
                std::stringstream s;
                auto stats = self.getStats();

                s << "group('" << self.getName() 
                << "', time=[" << STI::Utils::printTimeFormated(self.startTime())
                << ", " << STI::Utils::printTimeFormated(self.endTime())
                << "]"
                << ", subgroups=" << stats.subgroups 
                << ", events=" << stats.events 
                << ", vars=" << stats.vars 
                << ", tags=" << stats.tags 
                << ")";
                return s.str();
            })
        ;

}
