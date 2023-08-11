
#ifndef STI_PYTHON_REMOTEDEVICEPY_H
#define STI_PYTHON_REMOTEDEVICEPY_H


#include "DevicePy.h"

#include <memory>
#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

    class ChannelManagerPy;
    // class EventEngineSchedulerPy;
    class AttributeManagerPy;
    class PersistenceManagerPy;


    class RemoteDevicePy : public DevicePy
    {
    public:
        RemoteDevicePy() {}
        RemoteDevicePy(const std::shared_ptr<STI::Device::Device>& device);
        virtual ~RemoteDevicePy();



    private:

        std::shared_ptr<STI::Device::Device> device_;

    };




} //Python
} //STI

#endif

