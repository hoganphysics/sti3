%feature("director");



%{
    #include <sti/device/DeviceID.h>

    #include <sti/utils/Collector.h>
    #include <sti/utils/Collection.h>
    #include <sti/device/DeviceCollection.h>
    #include <sti/network/Node.h>
    #include <sti/device/Device.h>
    #include <sti/LocalDevice.h>
	#include "DeviceEventDispatcher.h"

// namespace STI 
// {
// namespace Network
// {
// class Node2;
// }
// }
//     using namespace STI::Device;
//     using namespace STI::Network;
%}

%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"

%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;
%rename(STI_Collection) STI::Utils::Collection;
%template(DeviceIDset) std::set< STI::Device::DeviceID >;



%shared_ptr(STI::Device::LocalDevice);
%shared_ptr(STI::Device::Device);
%shared_ptr(STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >);
%shared_ptr(STI::Utils::Collection< STI::Device::DeviceID,STI::Device::Device >);
%shared_ptr(STI::Utils::Collector< STI::Device::DeviceID,STI::Device::Device >);
%shared_ptr(STI::Device::DeviceEventDispatcher);
%shared_ptr(STI::Device::DeviceEventHandler);
%shared_ptr(STI::Device::DeviceEvent);
%shared_ptr(STI::Device::AbstractEventListenerGroup);


namespace STI {
namespace Device {
    class Device;
}
}



%include "DeviceID.h"
%include "Collector.h"
%rename(STI_Collection) STI::Utils::Collection;
%include "Collection.h"
%include "DeviceCollection.h"
%template(DeviceCollection) STI::Utils::Collection< STI::Device::DeviceID,STI::Device::Device >;
%template(DeviceCollector) STI::Utils::Collector< STI::Device::DeviceID,STI::Device::Device >;

%include "DeviceEventDispatcher.h"


//%include "Node.h"

namespace STI 
{

namespace Device 
{

class Device;

}
}

//renames both:
//%rename(DeviceNode_refresh) STI::Network::Node< STI::Device::DeviceID, STI::Device::Device>::refresh();

namespace STI 
{
namespace Network
{

template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>
{
public:
	virtual ~Node() {}
	//maybe node crawler hooks too?
	void ping() { return; }
	virtual bool refresh() = 0;
	//virtual T& get() = 0;
	T& get() { return static_cast<T&>(*this); }		//static polymorphism via CRTP

	T* operator->() { return &get(); }
};

//typedef DeviceNode

}


%template(DeviceNode) STI::Network::Node< STI::Device::DeviceID,STI::Device::Device >;
//%rename(DeviceNode_refresh) DeviceNode::refresh();



namespace Device 
{

class Device;

typedef STI::Network::Node< STI::Device::DeviceID, STI::Device::Device> DeviceNode2;

//%ignore STI::Network::Node< STI::Device::DeviceID, STI::Device::Device>;



class Device : public STI::Network::Node< STI::Device::DeviceID, STI::Device::Device>
//class Device : public DeviceNode2
{
public:
	virtual ~Device() {}
    //using DeviceNode::refresh();

	virtual void write(unsigned input) = 0;

	//virtual bool refresh() = 0;

	virtual void getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher) = 0;

};


} //Device
} //STI

%ignore DeviceCollectionPolicy;

%include "LocalDevice.h"


