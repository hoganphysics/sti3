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

    #include "utils/FileHolder.h"
    using STI::Utils::FileHolder;

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

    #include "ShotResult.h"
    

    // #include "JPersistenceManager.h"

    #include "JEventEngine.h"

%}


%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_pair.i"
%include "typemaps.i"

//***** Shared pointer definitions ********//

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::JLocalDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);
%shared_ptr(STI::Device::JDeviceMessageReceiver);
%shared_ptr(STI::Device::JDeviceMessageDispatcher);
%shared_ptr(STI::Engine::JEventEngineScheduler);
%shared_ptr(STI::Device::JChannelManager);
%shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::JPersistenceManager);

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
%shared_ptr(STI::Device::EngineParserDeviceMessage);
%shared_ptr(STI::Device::CollectionUpdateMessage);
%shared_ptr(STI::Device::EngineStateMessage);


// %ignore STI::Device::EngineJobUpdateDeviceMessage;
%shared_ptr(STI::Device::EngineJobUpdateDeviceMessage);
%shared_ptr(STI::Device::JEngineJobUpdateDeviceMessage);

//Event Engine Job
%shared_ptr(STI::Engine::JEventEngineJob);
// %shared_ptr(STI::Engine::EventEngineJob);

%shared_ptr(STI::Engine::EventEngineDependencyTree);

%shared_ptr(STI::Engine::JEventEngine);

%shared_ptr(STI::Utils::FileHolder);

////////////////////////////////////



//DeviceID
%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%rename(opEvaluate) operator();
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
%ignore STI::Device::JDeviceCollection::JDeviceCollection(std::shared_ptr< STI::Device::DeviceCollection >& collection);
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
%ignore STI::Device::EngineSchedulerMessage::setEngine(const std::shared_ptr< STI::Engine::EventEngine >& engine);
%ignore STI::Device::EngineSchedulerMessage::getEngine() const;
%ignore STI::Device::EngineJobUpdateDeviceMessage::toQueuedList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::toRunningList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
%ignore STI::Device::EngineJobUpdateDeviceMessage::toCompleteList(const std::shared_ptr< STI::Engine::EventEngineJob >& job);
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

//Attributes
%template(StringMap) std::map< std::string, std::string >;
%ignore STI::Device::AttributeManager;
%ignore STI::Device::JAttributeManager::JAttributeManager(std::shared_ptr< STI::Device::AttributeManager >& manager);
%include "JAttributeManager.h"



//JLocalDevice
%include "JLocalDevice.h"

//EngineJobID
%template(EngineJobIDSet) std::set< STI::Engine::EngineJobID >;
%include "EngineJobID.h"


//FileHolder
%ignore STI::Utils::FileHolder::write(const char* buffer, unsigned length);
%ignore STI::Utils::FileHolder::openFile();
%ignore STI::Utils::FileHolder::closeFile();
%include "utils/FileHolder.h"



//MixedValue
%warnfilter(516) STI::Utils::MixedValue::setValue;
%include "fwd/MixedValue_fwd.h"
%include "MixedValue.h"
%template(MixedValueVec) std::vector< STI::Utils::MixedValue >;
%include "MixedValue.h"
%rename(MixedValueVec) STI::Utils::MixedValueVector;





//JShot
%ignore STI::Engine::JShot::JShot(std::shared_ptr< STI::Engine::Shot >& shot);
%include "JShot.h"


// %nspace STI::Engine::ParseID
// %nspace STI::Engine::ShotID
// %nspace STI::Engine::TimeStamp



//JEventEngineScheduler
%ignore STI::Engine::EventEngineScheduler;
%ignore STI::Engine::JEventEngineScheduler::JEventEngineScheduler(const std::shared_ptr< STI::Engine::EventEngineScheduler >& scheduler);
%include "JEventEngineScheduler.h"

//EngineParsingMessage
%include "EngineParsingMessage.h"
%template(EngineParserMessageVector) std::vector< STI::Engine::EngineParsingMessage >;


//ChannelUpdateMessage
%template(ChannelMixedValueMap) std::map< short, STI::Utils::MixedValue >;

//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"

//HubID
%include "HubID.h"

//JNodeWalker
%ignore STI::Network::JNodeWalker::JNodeWalker(STI::Network::LocalDeviceHub::HubNodeWalker& root);
%ignore STI::Network::JHubGraphNode::JHubGraphNode(const STI::Network::DirectedGraphHub< STI::Device::DeviceID, STI::Device::Device >& hub);
%ignore STI::Network::JDeviceGraphNode::JDeviceGraphNode(const STI::Network::DirectedGraphNode< STI::Device::DeviceID, STI::Device::Device >& deviceNode);
%include "JNodeWalker.h"
%template(JNodeWalkerVector) std::vector< STI::Network::JNodeWalker >;
%template(JDeviceGraphNodeVector) std::vector< STI::Network::JDeviceGraphNode >;

