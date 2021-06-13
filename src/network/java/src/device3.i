%feature("director");

//%feature(nspace);

// %rename(Engine_ParseID) STI::Engine::ParseID;
// %rename(Engine_ShotID) STI::Engine::ShotID;
// %rename(Engine_TimeStamp) STI::Engine::TimeStamp;

// %warnfilter(401) STI::Device::DeviceCollection;
// %warnfilter(401) DeviceCollection;

%warnfilter(401);   //Warning 401:  Nothing known about base class 'STI::Device::DeviceCollection' (also STI::Device::Device)


%{
    #include "DeviceID.h"
    #include "Device.h"
    #include "JDevice.h"
    #include "JLocalDevice.h"    
    #include "DeviceMessage.h"
    #include "DeviceCollection.h"
    #include "JDeviceCollection.h"
    #include "JNetworkDeviceHub.h"
    #include "JNodeWalker.h"
    #include "HubID.h"

    #include "DeviceTrace.h"

    #include "DeviceMessageListener.h"
    #include "DeviceMessageReceiver.h"
    #include "DeviceMessageDispatcher.h"
    #include "JDeviceMessageReceiver.h"
    #include "JDeviceMessageDispatcher.h"
    #include "JEventEngineScheduler.h"
    using STI::Engine::JEventEngineScheduler;

    #include "TimeStamp.h"
    using STI::Engine::TimeStamp;

    #include "EngineJobID.h"
    #include "ParseID.h"
    using STI::Engine::ParseID;
    #include "ShotID.h"
    using STI::Engine::ShotID;
    #include "EngineID.h"
    using STI::Engine::EngineID;

    #include "EngineState.h"
    using STI::Engine::EngineState;

    #include "JShot.h"
    using STI::Engine::JShot;

    #include "RawEvent.h"
    using STI::Engine::RawEventType;
    #include "EventStackTrace.h"
    using STI::Engine::EventStackTrace;
    #include "utils/GraphPathLabel.h"
    using STI::Utils::GraphPathLabel;

    #include "EngineParsingMessage.h"
    using STI::Engine::EngineParsingMessage;

    #include "MixedValue.h"

    using STI::Utils::MixedValue;
    using STI::Utils::MixedValueType;
    using STI::Utils::MixedValueVector;

    #include "JChannelManager.h"
    #include "fwd/Channel_fwd.h"
    #include "Channel.h"
    #include "LocalChannel.h"
    using STI::Device::LocalChannel;

    #include "ChannelRefreshListener.h"
    using STI::Device::ChannelRefreshListener;

    #include "JAttributeManager.h"

%}


%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"
%include "std_vector.i"
%include "std_map.i"

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::JLocalDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);
%shared_ptr(STI::Device::JDeviceMessageReceiver);
%shared_ptr(STI::Device::JDeviceMessageDispatcher);
%shared_ptr(STI::Engine::JEventEngineScheduler);
%shared_ptr(STI::Device::JChannelManager);
%shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::Channel);
%shared_ptr(STI::Device::LocalChannel);

%shared_ptr(STI::Engine::JShot);

//%shared_ptr(STI::Device::DeviceMessageReceiver);

//Events
%shared_ptr(STI::Device::DeviceMessage);
%shared_ptr(STI::Device::RefreshDeviceMessage);
%shared_ptr(STI::Device::ChannelUpdateMessage);
%shared_ptr(STI::Device::AttributeUpdateMessage);
%shared_ptr(STI::Device::EngineSchedulerMessage);
// %shared_ptr(STI::Device::EngineParserMessage);
%shared_ptr(STI::Device::EventEngineMessage);
%shared_ptr(STI::Device::EngineParserDeviceMessage);
%shared_ptr(STI::Device::CollectionUpdateMessage);


//DeviceID
%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;
%include "DeviceID.h"
%template(DeviceIDset) std::set< STI::Device::DeviceID >;
%template(DeviceIDvector) std::vector< STI::Device::DeviceID >;

%include "DeviceTrace.h"

//EngineID
%include "EngineID.h"

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
%warnfilter(401) STI::Device::Device;
%include "JDevice.h"




//DeviceMessage
%include "DeviceMessage.h"
%include "DeviceMessageListener.h"

//Listeners
//Note: It's *very* important that the %shared_ptr definition comes before the %template call.

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::RefreshDeviceMessage >);
%template(RefreshDeviceMessageListener) STI::Device::DeviceMessageListener< STI::Device::RefreshDeviceMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::ChannelUpdateMessage >);
%template(ChannelUpdateMessageListener) STI::Device::DeviceMessageListener< STI::Device::ChannelUpdateMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::AttributeUpdateMessage >);
%template(AttributeUpdateMessageListener) STI::Device::DeviceMessageListener< STI::Device::AttributeUpdateMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineSchedulerMessage >);
%template(EngineSchedulerMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineSchedulerMessage >;

// %shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineParserMessage >);
// %template(EngineParserMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineParserMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EventEngineMessage >);
%template(EventEngineMessageListener) STI::Device::DeviceMessageListener< STI::Device::EventEngineMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineParserDeviceMessage >);
%template(EngineParserDeviceMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineParserDeviceMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::CollectionUpdateMessage >);
%template(CollectionUpdateMessageListener) STI::Device::DeviceMessageListener< STI::Device::CollectionUpdateMessage >;


//Event handling system
%ignore STI::Device::DeviceMessageReceiver;
%include "JDeviceMessageReceiver.h"

%include "JDeviceMessageDispatcher.h"

//ChannelManager
%include "fwd/Channel_fwd.h"
%include "Channel.h"
%template(ChannelVector) std::vector< std::shared_ptr < STI::Device::Channel > >;
%ignore STI::Device::ChannelManager;
%include "JChannelManager.h"

%include "LocalChannel.h"
%include "ChannelRefreshListener.h"


%include "TimeStamp.h"
%include "ParseID.h"
%include "ShotID.h"

//Attributes
%template(StringMap) std::map< std::string, std::string >;
%ignore STI::Device::AttributeManager;
%include "JAttributeManager.h"


//JLocalDevice
%include "JLocalDevice.h"

//EngineJobID
%include "EngineJobID.h"


//MixedValue
%warnfilter(516) STI::Utils::MixedValue::setValue;
%include "fwd/MixedValue_fwd.h"
%include "MixedValue.h"
%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;


%include "JShot.h"


// %nspace STI::Engine::ParseID
// %nspace STI::Engine::ShotID
// %nspace STI::Engine::TimeStamp


//JEventEngineScheduler
%ignore STI::Engine::EventEngineScheduler;
%include "JEventEngineScheduler.h"

//EngineParsingMessage
%include "EngineParsingMessage.h"
%template(EngineParserMessageVector) std::vector< STI::Engine::EngineParsingMessage >;

%include "EngineState.h"

//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"

//HubID
%include "HubID.h"

//JNodeWalker
%include "JNodeWalker.h"
%template(JNodeWalkerVector) std::vector< STI::Network::JNodeWalker >;
%template(JDeviceGraphNodeVector) std::vector< STI::Network::JDeviceGraphNode >;
