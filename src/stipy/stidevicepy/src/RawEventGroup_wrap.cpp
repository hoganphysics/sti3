#include <sti/engine/RawEventGroup.h>

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/ParsedTag.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/StackTraceData.h>
#include <sti/utils/utils.h>
#include <sti/utils/MetaData.h>

#include "MixedValuePy.h"
#include "StackTrace.h"

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using STI::Python::MixedValuePy;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventGroupStats;
using STI::Engine::RawEventType;
using STI::Engine::StackTrace;


void init_RawEventGroup(py::module& m)
{
    py::class_<RawEventGroupStats>(m, "RawEventGroupStats")
        .def(py::init<>())
        .def_readwrite("vars", &RawEventGroupStats::vars)
        .def_readwrite("tags", &RawEventGroupStats::tags)
        .def_readwrite("events", &RawEventGroupStats::events)
        .def_readwrite("subgroups", &RawEventGroupStats::subgroups)
        .def("__iadd__", &RawEventGroupStats::operator+=)
        .def("__repr__", [](const RawEventGroupStats& stats) {
            std::ostringstream oss;
            oss << "RawEventGroupStats("
                << "subgroups=" << stats.subgroups
                << ", events=" << stats.events
                << ", vars=" << stats.vars
                << ", tags=" << stats.tags
                << ")";
            return oss.str();
        })
    ;

    py::class_<RawEventGroup, std::shared_ptr<RawEventGroup>>(m, "RawEventGroup")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>(), py::arg("name"), py::arg("parentName"))
        
        .def("getName", py::overload_cast<>(&RawEventGroup::getName, py::const_))
        .def("getFullName", py::overload_cast<>(&RawEventGroup::getFullName, py::const_))
        .def("getParentGroupName", py::overload_cast<>(&RawEventGroup::getParentGroupName, py::const_))
        .def("setName", &RawEventGroup::setName)

        .def("getStats", &RawEventGroup::getStats)
        .def("getTotalStats", &RawEventGroup::getTotalStats)

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
        .def("getReferencePoints",
            [](const RawEventGroup& self) {
                auto refPoints = self.getReferencePoints();
                
                py::dict refDict;
                for (const auto& ref : refPoints) {
                    refDict[ref.first.c_str()] = ref.second;
                }
                return refDict;
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
        .def("metadata", [](RawEventGroup& self) {
                // MixedValuePy value(self.getMetaData());
                // return py::dict(value.getValue_py());
                auto& vec = self.getMetaData().getMetaData().getVector();
                py::dict values;
                
                for (auto& tuple : vec) {
                    MixedValuePy pyval(tuple.getVector().at(1));
                    // values[py::int_{tuple.first}] = pyval.getValue_py();
                    values[tuple.getVector().at(0).getString().c_str()] = pyval;
                }
                return values;
            })

        .def("var", &RawEventGroup::var, py::arg("fullVarName"), py::arg("stackTrace"))

        .def("_addvar", //name overriden to 'setvar' in python
            [](RawEventGroup& self, const std::string& fullVarName, 
                        const pybind11::object& value, const StackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                auto result = self.addvar(fullVarName, mixedValue, stackTrace);

                if (!result.success) {
                    throw std::runtime_error(result.errorMessage);
                }
            }, py::arg("fullVarName"), py::arg("value"), py::arg("stackTrace"))
        // .def("_addtag", //name overriden to 'settag' in python
        //     &RawEventGroup::addtag, py::arg("fullTagName"), py::arg("stackTrace"))
        .def("_addtag", //name overriden to 'settag' in python
            [](RawEventGroup& self, const std::string& fullTagName, const StackTrace& stackTrace) {

                auto result = self.addtag(fullTagName, stackTrace);

                if (!result.success) {
                    throw std::runtime_error(result.errorMessage);
                }
            }, py::arg("fullTagName"), py::arg("stackTrace"))
        .def("_addEvent",    //name overriden to 'event' in python
            [](RawEventGroup& self, const STI::Engine::RawEventTarget& target, 
                double time, const pybind11::object& value,
                const STI::Engine::StackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
            })
        .def("_addMeas", //name overriden to 'meas' in python
                [](RawEventGroup& self, const STI::Engine::RawEventTarget& target, 
                        double time, const pybind11::object& value,
                        const STI::Engine::StackTrace& stackTrace) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                self.addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
            }, py::arg("target"), py::arg("time"), py::arg("value"), py::arg("stackTrace"))

        .def("_addPostProcessRequest", //name overriden to 'postProcess' in python
            [](RawEventGroup& self, const STI::Engine::PostProcessTarget& target,
                const py::dict& options, const StackTrace& stackTrace) {

                STI::Utils::MetaData metaData;
                for (auto item : options) {
                    MixedValuePy value;
                    value.setValue_py(py::reinterpret_borrow<py::object>(item.second));
                    //Pass as the base MixedValue& so MetaData's non-template addMetaData
                    //overload is selected; the templated one would route the derived
                    //MixedValuePy through MixedValue's catch-all setValue<T> (prints an error).
                    metaData.addMetaData(py::str(item.first).cast<std::string>(),
                                         static_cast<const STI::Utils::MixedValue&>(value));
                }

                self.addPostProcessRequest(target, metaData, stackTrace);
            }, py::arg("target"), py::arg("options"), py::arg("stackTrace"))

        .def("bindvar", [](RawEventGroup& self, const std::string& fullVarName, 
                        const pybind11::object& value) {

                MixedValuePy mixedValue;
                mixedValue.setValue_py(value);

                auto result = self.bindVar(fullVarName, mixedValue);

                if (!result.success) {
                    throw std::runtime_error(result.errorMessage);
                }
            }, py::arg("fullVarName"), py::arg("value"))

        .def("group", &RawEventGroup::group, py::arg("name"))
        .def("subgroups", &RawEventGroup::getSubgroups)
        .def("addSubgroup", &RawEventGroup::addSubgroup, py::arg("subgroup"))

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
        .def("postProcessRequests", &RawEventGroup::postProcessRequests, py::return_value_policy::reference_internal)
        .def("overwrittenVars", &RawEventGroup::getOverwrittenVars)

        .def("getStackTraceData", &RawEventGroup::getStackTraceData)
        
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
