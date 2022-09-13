
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceID.h>

#include <memory>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Device::DeviceMessageType;
using STI::Device::DeviceMessage;
using STI::Device::RefreshDeviceMessage;



void init_DeviceMessage(py::module& m) 
{

    py::enum_<DeviceMessageType>(m, "DeviceMessageType")
        .value("Refresh", DeviceMessageType::Refresh)
        .value("CollectionUpdate", DeviceMessageType::CollectionUpdate)
        .value("ChannelUpdate", DeviceMessageType::ChannelUpdate)
        .value("ChannelsRefresh", DeviceMessageType::ChannelsRefresh)
        .value("AttributeUpdate", DeviceMessageType::AttributeUpdate)
        .value("AttributesRefresh", DeviceMessageType::AttributesRefresh)
        .value("MonitorUpdate", DeviceMessageType::MonitorUpdate)
        .value("EngineScheduler", DeviceMessageType::EngineScheduler)
        .value("EngineParser", DeviceMessageType::EngineParser)
        .value("EngineStatus", DeviceMessageType::EngineStatus)
        .value("Unknown", DeviceMessageType::Unknown)
        .export_values();


    py::class_<DeviceMessage, std::shared_ptr<DeviceMessage>>(m, "DeviceMessage")
        .def("sourceID", &DeviceMessage::sourceID)
        .def("getType", &DeviceMessage::getType)
        .def("__repr__",
            [](const DeviceMessage& mess) {
                return "<DeviceMessage | type=" 
                    + DeviceMessage::typeToString(mess.getType()) 
                    + ", source=" + mess.sourceID().getID() + ">";
            })
        ;


    py::class_<RefreshDeviceMessage, DeviceMessage, std::shared_ptr<RefreshDeviceMessage>>(m, "RefreshDeviceMessage")
        .def(py::init<const STI::Device::DeviceID&>(), py::arg("source") 
        );


}

