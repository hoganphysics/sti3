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

    #include "DeviceTrace.h"

    #include "DeviceMessageListener.h"
    #include "DeviceMessageReceiver.h"
    #include "DeviceMessageDispatcher.h"
    #include "JDeviceMessageReceiver.h"
    #include "JDeviceMessageDispatcher.h"

    #include "EventEngineJobList.h"
    using STI::Engine::EventEngineJobList;

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

    #include "JEngineJobUpdateDeviceMessage.h"
    #include "JEngineJobUpdateDeviceMessageListener.h"
    
    #include "EngineJobSourceID.h"
    using STI::Engine::EngineJobSourceID;

    #include "ShotConfig.h"
    using STI::Engine::ShotConfig;

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

    #include "JChannelManager.h"
    using STI::Device::JChannelManager;
    #include "fwd/Channel_fwd.h"
    #include "Channel.h"
    #include "LocalChannel.h"
    using STI::Device::LocalChannel;

    #include "ChannelRefreshListener.h"
    using STI::Device::ChannelRefreshListener;

    // #include "JAttributeManager.h"
    // #include "Attribute.h"
    // using STI::Device::Attribute;

    #include "ShotResult.h"
    

    // #include "JPersistenceManager.h"

    #include "JEventEngine.h"

%}


//***** Shared pointer definitions ********//

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::JLocalDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);
%shared_ptr(STI::Device::JDeviceMessageReceiver);
%shared_ptr(STI::Device::JDeviceMessageDispatcher);
%shared_ptr(STI::Engine::JEventEngineScheduler);
%shared_ptr(STI::Device::JChannelManager);
// %shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::JPersistenceManager);
%shared_ptr(STI::Device::Channel);
%shared_ptr(STI::Device::LocalChannel);
// %shared_ptr(STI::Device::Attribute);
%shared_ptr(STI::Engine::JShot);


//Messages
%shared_ptr(STI::Device::DeviceMessage);
%shared_ptr(STI::Device::RefreshDeviceMessage);
%shared_ptr(STI::Device::ChannelUpdateMessage);
%shared_ptr(STI::Device::AttributeUpdateMessage);
%shared_ptr(STI::Device::EngineSchedulerMessage);
%shared_ptr(STI::Device::EngineParserDeviceMessage);
%shared_ptr(STI::Device::CollectionUpdateMessage);
%shared_ptr(STI::Device::EngineStateMessage);
%shared_ptr(STI::Device::EngineJobUpdateDeviceMessage);
%shared_ptr(STI::Device::JEngineJobUpdateDeviceMessage);


%shared_ptr(STI::Engine::JEventEngineJob);
%shared_ptr(STI::Engine::EventEngineDependencyTree);
%shared_ptr(STI::Engine::JEventEngine);


////////////////////////////////////




%include "DeviceTrace.h"

//EngineID
%include "EngineID.h"

//EventEngineJobList
%include "EventEngineJobList.h"

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
%ignore STI::Device::JDeviceCollection::JDeviceCollection(const std::shared_ptr< STI::Device::DeviceCollection >& collection);
%include "JDeviceCollection.h"


//JDevice
%ignore STI::Device::Device;
%ignore STI::Device::JDevice::JDevice(const std::shared_ptr< STI::Device::Device >& device);
%warnfilter(401) STI::Device::Device;
%include "JDevice.h"


//EngineStateMessage
%template(EngineIDtoStateMap) std::map< STI::Engine::EngineID, STI::Engine::EngineState, std::less< STI::Engine::EngineID > >;
typedef std::map< STI::Engine::EngineID, STI::Engine::EngineState, std::less< STI::Engine::EngineID > >::iterator EngineIDtoStateMapIterator;


//shared_ptr declaration must be before %include call
%shared_ptr(STI::Device::JEngineJobUpdateDeviceMessageListener);


//DeviceMessage
// %ignore STI::Device::EngineJobUpdateDeviceMessage;
%ignore STI::Device::EngineJobUpdateDeviceMessage::EngineJobUpdateDeviceMessage(const STI::Device::DeviceTrace& trace, const std::shared_ptr< STI::Engine::EventEngineJob >& job, STI::Engine::EventEngineJobList targetList);
%ignore STI::Device::EngineSchedulerMessage::setEngine(const std::shared_ptr< STI::Engine::EventEngine >& engine);
%ignore STI::Device::EngineSchedulerMessage::getEngine() const;
%ignore STI::Device::EngineJobUpdateDeviceMessage::toQueuedList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::toRunningList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::toCompleteList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::toArchive(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::getEngineJob() const;
%include "DeviceMessage.h"

%include "DeviceMessageListener.h"

%ignore STI::Device::DeviceMessageListener< STI::Device::EngineJobUpdateDeviceMessage >::handleMessage;

%extend STI::Device::EngineSchedulerMessage 
{
    std::shared_ptr< STI::Engine::JEventEngine > STI::Device::EngineSchedulerMessage::getJEventEngine() const
    {
        auto jEventEngine = std::make_shared< STI::Engine::JEventEngine >(self->getEngine());
        return jEventEngine;
    }
} 

%extend STI::Device::EngineJobUpdateDeviceMessage 
{
    std::shared_ptr< STI::Engine::JEventEngineJob > STI::Device::EngineJobUpdateDeviceMessage::getJEventEngineJob() const
    {
        auto jEventEngineJob = std::make_shared< STI::Engine::JEventEngineJob >(self->getEngineJob());
        return jEventEngineJob;
    }
}



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

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineParserDeviceMessage >);
%template(EngineParserDeviceMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineParserDeviceMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::CollectionUpdateMessage >);
%template(CollectionUpdateMessageListener) STI::Device::DeviceMessageListener< STI::Device::CollectionUpdateMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineStateMessage >);
%template(EngineStateMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineStateMessage >;

%shared_ptr(STI::Device::DeviceMessageListener< STI::Device::EngineJobUpdateDeviceMessage >);
%template(EngineJobUpdateDeviceMessageListener) STI::Device::DeviceMessageListener< STI::Device::EngineJobUpdateDeviceMessage >;

%include "JEngineJobUpdateDeviceMessage.h"


// %rename(handleMessage33) STI::Device::JEngineJobUpdateDeviceMessageListener::handleJMessage(const std::shared_ptr< STI::Device::JEngineJobUpdateDeviceMessage >&);

%rename(handleMessage) STI::Device::JEngineJobUpdateDeviceMessageListener::handleJMessage;
%include "JEngineJobUpdateDeviceMessageListener.h"

// %ignore STI::Device::JEngineJobUpdateDeviceMessageListener::handleMessage(const std::shared_ptr< STI::Device::EngineJobUpdateDeviceMessage >& mess);



//Message handling system
%ignore STI::Device::DeviceMessageReceiver;
%ignore STI::Device::JDeviceMessageReceiver::JDeviceMessageReceiver(std::shared_ptr< STI::Device::DeviceMessageReceiver >& receiver);
%include "JDeviceMessageReceiver.h"

//JDeviceMessageDispatcher
%ignore STI::Device::JDeviceMessageDispatcher::JDeviceMessageDispatcher(std::shared_ptr< STI::Device::DeviceMessageDispatcher >& dispatcher);
%include "JDeviceMessageDispatcher.h"


//ChannelManager
%include "fwd/Channel_fwd.h"
%include "Channel.h"
%template(ChannelVector) std::vector< std::shared_ptr < STI::Device::Channel > >;
%ignore STI::Device::ChannelManager;
%ignore STI::Device::JChannelManager::JChannelManager(std::shared_ptr< STI::Device::ChannelManager >& manager);
%include "JChannelManager.h"

%include "LocalChannel.h"
%include "ChannelRefreshListener.h"

//EngineJobSourceID
%include "EngineJobSourceID.h"

//ShotConfig
%include "ShotConfig.h"


%include "TimeStamp.h"
%include "ParseID.h"
%include "ShotID.h"

// //Attributes
// %template(StringMap) std::map< std::string, std::string >;
// %template(StringVector) std::vector< std::string >;
// %include "Attribute.h"
// %template(AttributeVector) std::vector< std::shared_ptr < STI::Device::Attribute > >;
// %ignore STI::Device::AttributeManager;
// %ignore STI::Device::JAttributeManager::JAttributeManager(std::shared_ptr< STI::Device::AttributeManager >& manager);
// %include "JAttributeManager.h"



//JLocalDevice
%include "JLocalDevice.h"

//EngineJobID
%template(EngineJobIDSet) std::set< STI::Engine::EngineJobID >;
%include "EngineJobID.h"







//JShot
%ignore STI::Engine::JShot::JShot(std::shared_ptr< STI::Engine::Shot >& shot);
%include "JShot.h"


// %nspace STI::Engine::ParseID
// %nspace STI::Engine::ShotID
// %nspace STI::Engine::TimeStamp



//EngineParsingMessage
%include "EngineParsingMessage.h"
%template(EngineParserMessageVector) std::vector< STI::Engine::EngineParsingMessage >;


//ChannelUpdateMessage
%template(ChannelMixedValueMap) std::map< short, STI::Utils::MixedValue >;
