

#include"DeviceMessageReceiverPy.h"
#include <sti/device/DeviceMessage.h>

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
namespace py = pybind11;

using STI::Python::DeviceMessageReceiverPy;


void init_DeviceMessageReceiver(py::module& m) 
{

    // Refresh, CollectionUpdate, 
	// ChannelUpdate, ChannelsRefresh, 
	// AttributeUpdate, AttributesRefresh, 
	// MonitorUpdate, 
	// EngineJobUpdate,
	// EngineScheduler, 
	// EngineParser,
	// EngineStatus,
	// Unknown 

    py::class_<DeviceMessageReceiverPy, std::shared_ptr<DeviceMessageReceiverPy>>(m, "DeviceMessageReceiver")
        .def("__addRefreshListener", &DeviceMessageReceiverPy::addRefreshListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addCollectionUpdateListener", &DeviceMessageReceiverPy::addCollectionUpdateListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addChannelUpdateListener", &DeviceMessageReceiverPy::addChannelUpdateListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addAttributeUpdateListener", &DeviceMessageReceiverPy::addAttributeUpdateListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addMonitorUpdateListener", &DeviceMessageReceiverPy::addMonitorUpdateListener,
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addMonitorStatusUpdateListener", &DeviceMessageReceiverPy::addMonitorStatusUpdateListener,
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addTaskUpdateListener", &DeviceMessageReceiverPy::addTaskUpdateListener,
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addEngineJobUpdateDeviceListener", &DeviceMessageReceiverPy::addEngineJobUpdateListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addEngineSchedulerMessageListener", &DeviceMessageReceiverPy::addEngineSchedulerMessageListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("__addEngineStateMessageListener", &DeviceMessageReceiverPy::addEngineStateMessageListener, 
                                            py::arg("sourceDeviceID"), py::arg("listenerName"), py::arg("handler") )
        .def("removeListener", &DeviceMessageReceiverPy::removeListener, py::arg("sourceDeviceID"), py::arg("type"), py::arg("listenerName") )
        .def("clearListeners", &DeviceMessageReceiverPy::clearListeners)
        ;

}
