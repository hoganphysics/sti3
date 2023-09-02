

#include <sti/device/Channel.h>
#include <sti/device/LocalChannel.h>
#include "MixedValuePy.h"
#include <sti/utils/utils.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>

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
                return value.getValue_py();
            })
//        .def("getMetaData", &Channel::getMetaData)
        // .def("getMetaData", py::overload_cast<>(&Channel::getMetaData, py::const_))
        // .def("getMetaData", py::overload_cast<const std::string&>(&Channel::getMetaData, py::const_))
        .def("metadata", [](Channel& self) {
                MixedValuePy value(self.getMetaData());
                return value.getValue_py();
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
            } ) //, py::return_value_policy::reference)
        ;


}

