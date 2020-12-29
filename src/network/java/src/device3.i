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

    #include "ParsedShot.h"
    using STI::Engine::ParsedShot;

    #include "RawEvent.h"
    using STI::Engine::RawEventType;
    #include "EventStackTrace.h"
    using STI::Engine::EventStackTrace;
    #include "utils/GraphPathLabel.h"
    using STI::Utils::GraphPathLabel;

    #include "MixedValue.h"

    using STI::Utils::MixedValue;
    using STI::Utils::MixedValueType;
    using STI::Utils::MixedValueVector;

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

%shared_ptr(STI::Engine::ParsedShot);

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

//RawEvent
%include "fwd/RawEvent_fwd.h"
%template(UIntVector) std::vector< unsigned >;
%include "utils/GraphPathLabel.h"
%rename(UIntVector) STI::Utils::GraphPathLabel;
%include "EventStackTrace.h"
%include "RawEvent.h"
%template(RawEventVector) std::vector< STI::Engine::RawEvent >;
%shared_ptr( std::vector< STI::Engine::RawEvent > );



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


//MixedValue
%include "fwd/MixedValue_fwd.h"
%include "MixedValue.h"
%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;


%include "ParsedShot.h"


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