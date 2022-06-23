//%module(directors="1") example
%feature("director");

%include "std_string.i"
%include "std_shared_ptr.i"

%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;

// %{
//     #include <sti/device/DeviceID.h>
// %}
// %include "DeviceID.h"

// class Dog
// {

// };



%{

    #include <sti/device/DeviceID.h>

    #include <sti/utils/Collector.h>
    #include <sti/utils/Collection.h>
    #include <sti/device/DeviceCollection.h>
    //#include <sti/network/Node.h>
    //#include <sti/device/Device.h>
    using namespace STI::Device;
%}


namespace STI {
namespace Device {
    class Device;
}
}

%include "DeviceID.h"
%include "Collector.h"
%include "Collection.h"
%include "DeviceCollection.h"
%template(DeviceCollector) STI::Utils::Collector< STI::Device::DeviceID,STI::Device::Device >;
%template(DeviceCollection) STI::Utils::Collection< STI::Device::DeviceID,STI::Device::Device >;
%include "Node.h"
//#include <sti/network/Node.h>
//%template(DeviceNode) STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >;
//%rename(DeviceNodeX) STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >;
//typedef STI::Network::Node< STI::Device::DeviceID,STI::Device::Device > DeviceNode;

%template(DeviceNodeBase) STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >;

//#include "DeviceNode.h"
//Not sure why, but the above #include doesn't work.  I need to explicitly include the DeviceNode
//class definition here...  This is a work around.  Not too bad, since the class is
//essenential just a typedef to help SWIG in the first place.

namespace STI {
namespace Device {


class DeviceNode : public STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >
{
public:
	virtual ~DeviceNode() {}
};

}
}

#include <sti/device/Device.h>
%include "Device.h"



