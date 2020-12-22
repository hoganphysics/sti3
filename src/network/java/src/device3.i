%feature("director");

//%feature(nspace);

// %rename(Engine_ParseID) STI::Engine::ParseID;
// %rename(Engine_ShotID) STI::Engine::ShotID;
// %rename(Engine_TimeStamp) STI::Engine::TimeStamp;


%{
    #include "DeviceID.h"
    #include "Device.h"
    #include "JDevice.h"
    #include "JLocalDevice.h"    
    #include "DeviceEvent.h"
    #include "DeviceCollection.h"
    #include "JDeviceCollection.h"
    #include "JNetworkDeviceHub.h"
    #include "JNodeWalker.h"
    #include "HubID.h"

    #include "DeviceEventListener.h"
    #include "DeviceEventReceiver.h"
    #include "DeviceEventDispatcher.h"
    #include "JDeviceEventReceiver.h"
    #include "JDeviceEventDispatcher.h"
    #include "JEventEngineScheduler.h"

    #include "TimeStamp.h"
    using STI::Engine::TimeStamp;

    #include "EngineJobID.h"
    #include "ParseID.h"
    using STI::Engine::ParseID;
    #include "ShotID.h"
    using STI::Engine::ShotID;


%}

%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"
%include "std_vector.i"

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::JLocalDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);
%shared_ptr(STI::Device::JDeviceEventReceiver);
%shared_ptr(STI::Device::JDeviceEventDispatcher);
%shared_ptr(STI::Device::JEventEngineScheduler);

//%shared_ptr(STI::Device::DeviceEventReceiver);

//Events
%shared_ptr(STI::Device::DeviceEvent);
%shared_ptr(STI::Device::RefreshDeviceEvent);
%shared_ptr(STI::Device::ChannelUpdateDeviceEvent);
%shared_ptr(STI::Device::EngineSchedulerMessage);
%shared_ptr(STI::Device::EngineParserMessage);
%shared_ptr(STI::Device::EventEngineMessage);


//DeviceID
%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;
%include "DeviceID.h"
%template(DeviceIDset) std::set< STI::Device::DeviceID >;
%template(DeviceIDvector) std::vector< STI::Device::DeviceID >;

//JDeviceCollection
%ignore STI::Device::DeviceCollection;
%include "JDeviceCollection.h"


//JDevice
%ignore STI::Device::Device;
%include "JDevice.h"




//JLocalDevice
%include "JLocalDevice.h"

//DeviceEvent
%include "DeviceEvent.h"
%include "DeviceEventListener.h"

//Listeners
//Note: It's *very* important that the %shared_ptr definition comes before the %template call.

%shared_ptr(STI::Device::DeviceEventListener< STI::Device::RefreshDeviceEvent >);
%template(RefreshDeviceEventListener) STI::Device::DeviceEventListener< STI::Device::RefreshDeviceEvent >;

%shared_ptr(STI::Device::DeviceEventListener< STI::Device::ChannelUpdateDeviceEvent >);
%template(ChannelUpdateDeviceEventListener) STI::Device::DeviceEventListener< STI::Device::ChannelUpdateDeviceEvent >;

%shared_ptr(STI::Device::DeviceEventListener< STI::Device::EngineSchedulerMessage >);
%template(EngineSchedulerMessageListener) STI::Device::DeviceEventListener< STI::Device::EngineSchedulerMessage >;

%shared_ptr(STI::Device::DeviceEventListener< STI::Device::EngineParserMessage >);
%template(EngineParserMessageListener) STI::Device::DeviceEventListener< STI::Device::EngineParserMessage >;

%shared_ptr(STI::Device::DeviceEventListener< STI::Device::EventEngineMessage >);
%template(EventEngineMessageListener) STI::Device::DeviceEventListener< STI::Device::EventEngineMessage >;


//Event handling system
%ignore STI::Device::DeviceEventReceiver;
%include "JDeviceEventReceiver.h"

%include "JDeviceEventDispatcher.h"

%include "TimeStamp.h"
%include "ParseID.h"
%include "ShotID.h"

//EngineJobID
%include "EngineJobID.h"





// %nspace STI::Engine::ParseID
// %nspace STI::Engine::ShotID
// %nspace STI::Engine::TimeStamp


//JEventEngineScheduler
%ignore STI::Engine::EventEngineScheduler;
%include "JEventEngineScheduler.h"


//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"

//HubID
%include "HubID.h"

//JNodeWalker
%include "JNodeWalker.h"
%template(JNodeWalkerVector) std::vector< STI::Network::JNodeWalker >;
%template(JDeviceGraphNodeVector) std::vector< STI::Network::JDeviceGraphNode >;