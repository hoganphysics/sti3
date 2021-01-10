

#include "Channel.h"
#include "LocalChannel.h"


#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Device::Channel;
using STI::Device::LocalChannel;


void init_Channel(py::module& m) 
{

	// virtual short getChannelNumber() const = 0;

	// virtual STI::Device::ChannelType getType() const = 0;
	// virtual STI::Utils::MixedValueType getInputType() const = 0;
	// virtual STI::Utils::MixedValueType getOutputType() const = 0;

	// virtual void setChannelName(const std::string& name) = 0;
	// virtual std::string getChannelName() const = 0;

	// virtual void saveLastValue(const STI::Utils::MixedValue& value) = 0;
	// virtual const STI::Utils::MixedValue getLastValue() const = 0;

	// virtual const STI::Utils::MixedValue& getMetaData() const = 0;
	// virtual STI::Utils::MixedValue getMetaData(const std::string& key) const = 0;

    py::class_<Channel>(m, "Channel")
        .def("getChannelNumber", &Channel::getChannelNumber)
        .def("getType", &Channel::getType)
        .def("getInputType", &Channel::getInputType)
        .def("getOutputType", &Channel::getOutputType)
        .def("setChannelName", &Channel::setChannelName)
        .def("getChannelName", &Channel::getChannelName)
        //.def("getMetaData", &Channel::getMetaData)
        ;

    py::class_<LocalChannel, Channel>(m, "LocalChannel")
        //.def("addMetaData", &LocalChannel::addMetaData)
        ;

}

