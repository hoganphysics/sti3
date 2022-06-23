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

class Dog
{

};

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
%template(DeviceCollector) STI::Utils::Collector< DeviceID,Dog >;
%template(DeviceCollection) STI::Utils::Collection< DeviceID,Dog >;
%include "Node.h"
#include <sti/network/Node.h>
%template(DeviceNodeX) STI::Network::Node< DeviceID,Dog >;
//%rename(DeviceNodeX) STI::Network::Node< DeviceID,Device >;
//typedef STI::Network::Node< DeviceID, Device > DeviceNode;

class DeviceNode : public STI::Network::Node<DeviceID, Dog>
{
public:
	virtual ~DeviceNode() {}
};


#include <sti/device/Device.h>
//%include "Device.h"



