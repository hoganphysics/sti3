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
%}
%include "DeviceID.h"
%include "Collector.h"
%include "Collection.h"
%include "DeviceCollection.h"
%template(DeviceCollector) STI::Utils::Collector< DeviceID,Device >;
%template(DeviceCollection) STI::Utils::Collection< DeviceID,Device >;
%include "Node.h"
//#include <sti/network/Node.h>
//%template(DeviceNode) STI::Network::Node< DeviceID,Device >;
//%rename(DeviceNodeX) STI::Network::Node< DeviceID,Device >;
//typedef STI::Network::Node< DeviceID, Device > DeviceNode;
%template(DeviceNodeBase) STI::Network::Node< DeviceID,Device >;

//#include "DeviceNode.h"
//Not sure why, but the above #include doesn't work.  I need to explicitly include the DeviceNode
//class definition here...  This is a work around.  Not too bad, since the class is
//essenential just a typedef to help SWIG in the first place.

class DeviceNode : public STI::Network::Node<DeviceID, Device>
{
public:
	virtual ~DeviceNode() {}
};



#include <sti/device/Device.h>
%include "Device.h"



