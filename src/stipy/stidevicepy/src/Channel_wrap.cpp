

#include <sti/device/Channel.h>
#include <sti/device/LocalChannel.h>
#include "MixedValuePy.h"
#include <sti/utils/utils.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Device::Channel;
using STI::Device::LocalChannel;

using STI::Utils::MixedValue;
using STI::Python::MixedValuePy;

using STI::Device::ChannelType;
using STI::Utils::MixedValueType;

void init_Channel(py::module& m) 
{

    py::enum_<ChannelType>(m, "ChannelType")
        .value("Output", ChannelType::Output)
        .value("Input", ChannelType::Input)
        ;
        //.export_values();



    py::class_<Channel, std::shared_ptr<Channel>>(m, "Channel")
        .def("number", &Channel::getChannelNumber)
        .def("type", &Channel::getType)
        .def("inputType", &Channel::getInputType)
        .def("outputType", &Channel::getOutputType)
        .def("setName", &Channel::setChannelName)
        .def("name", &Channel::getChannelName)
        .def("getLastValue", [](Channel& self) {
                MixedValuePy value(self.getLastValue());
                return value;
            })
//        .def("getMetaData", &Channel::getMetaData)
        // .def("getMetaData", py::overload_cast<>(&Channel::getMetaData, py::const_))
        // .def("getMetaData", py::overload_cast<const std::string&>(&Channel::getMetaData, py::const_))
        // .def("metadata", [](Channel& self) {
        //         MixedValuePy value(self.getMetaData());
        //         return value.getValue_py();
        //     })
        .def("metadata", [](Channel& self) {
                // MixedValuePy value(self.getMetaData());
                // return py::dict(value.getValue_py());
                auto& vec = self.getMetaData().getVector();
                py::dict values;
                
                for (auto& tuple : vec) {
                    MixedValuePy pyval(tuple.getVector().at(1));
                    // values[py::int_{tuple.first}] = pyval.getValue_py();
                    values[tuple.getVector().at(0).getString().c_str()] = pyval;
                }
                return values;
            })
        .def("metadata", [](Channel& self, const std::string& key) {
                MixedValuePy value(self.getMetaData(key));
                return value.getValue_py();
            })
        .def("__repr__",
            [](const Channel& ch) {
                return "<ch=" + STI::Utils::valueToString(ch.getChannelNumber())
                    + "|name=" + ch.getChannelName()
                    + "|type=" + Channel::typeToString(ch.getType()) 
                    + "|in=" + MixedValue::TypeToString(ch.getInputType())
                    + "|out=" + MixedValue::TypeToString(ch.getOutputType()) + ">";
            })
        ;

    py::class_<LocalChannel, Channel, std::shared_ptr<LocalChannel>>(m, "LocalChannel")
        .def(py::init<>())
//        .def("addMetaData", &LocalChannel::addMetaData, py::return_value_policy::reference)
        .def("addMetadata", 
            [](LocalChannel& self, const std::string& key, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                //LocalChannel& ch = self.addMetaData(key, v);
                //return ch;        //error: use of deleted function ‘STI::Device::LocalChannel::LocalChannel(const STI::Device::LocalChannel&’
                self.addMetaData(key, v);
                return;
            }, py::arg("key"), py::arg("value") ) //, py::return_value_policy::reference)
        .def("setColor", 
            
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const std::string& color) {
                self->setColor(color);
                return self;
            }, py::arg("color") )
        .def("setUnits", 
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const std::string& units) {
                self->setUnits(units);
                return self;
            }, py::arg("units") )
        .def("setMinValue", 
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                self->setMinValue(v);
                return self;
            }, py::arg("minValue") )
        .def("setMaxValue", 
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                self->setMaxValue(v);
                return self;
            }, py::arg("maxValue") )
        .def("setVectorFormat", 
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const std::vector<STI::Utils::MixedValueType>& types) {
                self->setVectorFormat(types);
                return self;
            }, py::arg("types"))
        .def("setValueHint", 
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const std::string& hint) {
                self->setValueHint(hint);
                return self;
            }, py::arg("hint") )
        .def("setHelp",
            [](std::shared_ptr<STI::Device::LocalChannel>& self, const std::string& help) {
                self->setHelp(help);
                return self;
            }, py::arg("help") )
        ;


}

