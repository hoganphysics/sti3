

#include "Channel.h"
#include "LocalChannel.h"
#include "MixedValuePy.h"

#include <string>
#include <memory>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Device::Channel;
using STI::Device::LocalChannel;

using STI::Utils::MixedValue;
using STI::Python::MixedValuePy;

void init_Channel(py::module& m) 
{

    py::class_<Channel, std::shared_ptr<Channel>>(m, "Channel")
        .def("getChannelNumber", &Channel::getChannelNumber)
        .def("getType", &Channel::getType)
        .def("getInputType", &Channel::getInputType)
        .def("getOutputType", &Channel::getOutputType)
        .def("setChannelName", &Channel::setChannelName)
        .def("getChannelName", &Channel::getChannelName)
        .def("getLastValue", [](Channel& self) {
                MixedValuePy value(self.getLastValue());
                return value.getValue_py();
            })
//        .def("getMetaData", &Channel::getMetaData)
        // .def("getMetaData", py::overload_cast<>(&Channel::getMetaData, py::const_))
        // .def("getMetaData", py::overload_cast<const std::string&>(&Channel::getMetaData, py::const_))
        .def("getMetaData", [](Channel& self) {
                MixedValuePy value(self.getMetaData());
                return value.getValue_py();
            })
        .def("getMetaData", [](Channel& self, const std::string& key) {
                MixedValuePy value(self.getMetaData(key));
                return value.getValue_py();
            })
        ;

    py::class_<LocalChannel, Channel, std::shared_ptr<LocalChannel>>(m, "LocalChannel")
        .def(py::init<>())
//        .def("addMetaData", &LocalChannel::addMetaData, py::return_value_policy::reference)
        .def("addMetaData", 
            [](LocalChannel& self, const std::string& key, const MixedValuePy& value) {
                const MixedValue& v = static_cast<const MixedValue&>(value);
                //LocalChannel& ch = self.addMetaData(key, v);
                //return ch;        //error: use of deleted function ‘STI::Device::LocalChannel::LocalChannel(const STI::Device::LocalChannel&’
                self.addMetaData(key, v);
                return;
            } ) //, py::return_value_policy::reference)
        ;


}

