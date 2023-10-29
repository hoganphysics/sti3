
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEventGroup.h>

#include "MixedValuePy.h"

#include <memory>
#include <map>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Device::DeviceMessageType;
using STI::Device::DeviceMessage;
using STI::Device::RefreshDeviceMessage;
using STI::Device::CollectionUpdateMessage;
using STI::Device::ChannelUpdateMessage;
using STI::Device::AttributeUpdateMessage;
using STI::Device::EngineJobUpdateDeviceMessage;
using STI::Device::EngineSchedulerMessage;
using STI::Device::EngineStateMessage;
using STI::Python::MixedValuePy;

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
        ;

    py::class_<DeviceMessage, std::shared_ptr<DeviceMessage>>(m, "DeviceMessage")
        .def("sourceID", &DeviceMessage::sourceID)
        .def("originalSourceID", &DeviceMessage::originalSourceID)
        .def("type", &DeviceMessage::getType)
        .def("getDeviceTrace",
            [](const DeviceMessage& mess) {
                const auto& trace = mess.getDeviceTrace();
                
                std::vector<STI::Device::DeviceID> ids = trace.getIDs();
                return ids;
            })
        .def("__repr__",
            [](const DeviceMessage& mess) {
                return "<DeviceMessage | type=" 
                    + DeviceMessage::typeToString(mess.getType()) 
                    + ", source=" + mess.sourceID().getID() + ">";
            })
        ;

    py::class_<RefreshDeviceMessage, DeviceMessage, std::shared_ptr<RefreshDeviceMessage>>(m, "RefreshDeviceMessage")
        .def(py::init<const STI::Device::DeviceID&>(), py::arg("source"))
        ;
        
    // Add, Remove, Refresh
    py::enum_<CollectionUpdateMessage::CollectionMessageType>(m, "CollectionMessageType")
        .value("Add", CollectionUpdateMessage::CollectionMessageType::Add)
        .value("Remove", CollectionUpdateMessage::CollectionMessageType::Remove)
        .value("Refresh", CollectionUpdateMessage::CollectionMessageType::Refresh)
        ;

    py::class_<CollectionUpdateMessage, DeviceMessage, std::shared_ptr<CollectionUpdateMessage>>(m, "CollectionUpdateMessage")
        // .def(py::init<const STI::Device::DeviceID&>(), py::arg("source"))
        .def_readonly("updateType", &CollectionUpdateMessage::updateType)
        ;

    py::enum_<ChannelUpdateMessage::ChannelUpdateMessageType>(m, "ChannelUpdateMessageType")
        .value("ChannelValue", ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue)
        .value("ChannelName", ChannelUpdateMessage::ChannelUpdateMessageType::ChannelName)
        ;

    py::class_<ChannelUpdateMessage, DeviceMessage, std::shared_ptr<ChannelUpdateMessage>>(m, "ChannelUpdateMessage")
        // .def(py::init<const STI::Device::DeviceID&>(), py::arg("source") )
        .def_readonly("channelUpdateType", &ChannelUpdateMessage::channelUpdateType)
        // .def_readonly("channelValues", &ChannelUpdateMessage::channelValues)
        .def("channelValues",
            [](const ChannelUpdateMessage& mess) {
                py::dict values;
                
                for (auto& tuple : mess.channelValues) {
                    MixedValuePy pyval(tuple.second);
                    values[py::int_{tuple.first}] = pyval.getValue_py();
                }
                return values;
            })
        .def_readonly("channelNumber", &ChannelUpdateMessage::channelNumber, "Only for ChannelName messages")
        .def_readonly("channelName", &ChannelUpdateMessage::channelName)
        ;

    py::class_<AttributeUpdateMessage, DeviceMessage, std::shared_ptr<AttributeUpdateMessage>>(m, "AttributeUpdateMessage")
        .def_readonly("attributes", &AttributeUpdateMessage::attributes)
        ;

    py::class_<EngineJobUpdateDeviceMessage, DeviceMessage, std::shared_ptr<EngineJobUpdateDeviceMessage>>(m, "EngineJobUpdateDeviceMessage")
        .def("targetList", &EngineJobUpdateDeviceMessage::getTargetList)
        .def("engineJob", &EngineJobUpdateDeviceMessage::getEngineJob)
        ;


    //SchedulerMessageType { ParseComplete, YieldParse, PartialParse, PlayReady, PlayComplete, YieldPlay };
    py::enum_<EngineSchedulerMessage::SchedulerMessageType>(m, "SchedulerMessageType")
        .value("ParseComplete", EngineSchedulerMessage::SchedulerMessageType::ParseComplete)
        .value("YieldParse", EngineSchedulerMessage::SchedulerMessageType::YieldParse)
        .value("PartialParse", EngineSchedulerMessage::SchedulerMessageType::PartialParse)
        .value("PlayReady", EngineSchedulerMessage::SchedulerMessageType::PlayReady)
        .value("PlayComplete", EngineSchedulerMessage::SchedulerMessageType::PlayComplete)
        .value("YieldPlay", EngineSchedulerMessage::SchedulerMessageType::YieldPlay)
        ;

    py::class_<EngineSchedulerMessage, DeviceMessage, std::shared_ptr<EngineSchedulerMessage>>(m, "EngineSchedulerMessage")
        .def_readonly("schedulerMessageType", &EngineSchedulerMessage::schedulerMessageType)
        .def_readonly("jobID", &EngineSchedulerMessage::jobID)
        .def_readonly("handledEvents", &EngineSchedulerMessage::handledEvents)
        .def_readonly("unhandledEvents", &EngineSchedulerMessage::unhandledEvents)
        .def_readonly("upstreamPartnerEvents", &EngineSchedulerMessage::upstreamPartnerEvents)    
        .def_readonly("messages", &EngineSchedulerMessage::messages)
        .def_readonly("engineState", &EngineSchedulerMessage::engineState)
        ;

    py::class_<EngineStateMessage, DeviceMessage, std::shared_ptr<EngineStateMessage>>(m, "EngineStateMessage")
        .def_readonly("engineStates", &EngineStateMessage::engineStates)
        ;

}

