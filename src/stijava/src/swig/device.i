%feature("director");


%warnfilter(401);   //Warning 401:  Nothing known about base class 'STI::Device::DeviceCollection' (also STI::Device::Device)


%{

    #include <sti/device/Device.h>
    #include "JDevice.h"
    #include "JLocalDevice.h"    
    #include <sti/device/DeviceMessageType.h>
    #include <sti/device/DeviceMessage.h>
    #include <sti/device/DeviceCollection.h>
    #include "JDeviceCollection.h"


    #include <sti/utils/LocalCollection.h>
    using STI::Utils::LocalCollection;

    #include <sti/engine/SynchronousEvent.h>
    using STI::Engine::SynchronousEvent;
    using STI::Engine::SynchronousEventAdapter;

    #include <sti/device/DeviceTrace.h>

    #include <sti/device/DeviceMessageListener.h>
    #include <sti/device/DeviceMessageReceiver.h>
    #include <sti/device/DeviceMessageDispatcher.h>
    #include "JDeviceMessageReceiver.h"
    #include "JDeviceMessageDispatcher.h"


    #include <sti/engine/EventEngineJobList.h>
    using STI::Engine::EventEngineJobList;

    #include <sti/engine/EngineJobStatus.h>
    using STI::Engine::EngineJobStatus;

    #include <sti/engine/TimeStamp.h>
    using STI::Engine::TimeStamp;

    #include <sti/engine/EngineJobID.h>
    #include <sti/engine/ParseID.h>
    using STI::Engine::ParseID;
    #include <sti/engine/ShotID.h>
    using STI::Engine::ShotID;
    #include <sti/engine/EngineID.h>
    using STI::Engine::EngineID;

    #include <sti/engine/EngineState.h>
    using STI::Engine::EngineState;

    #include "JEngineJobUpdateDeviceMessage.h"
    #include "JEngineJobUpdateDeviceMessageListener.h"
    
    #include <sti/engine/EngineJobSourceID.h>
    using STI::Engine::EngineJobSourceID;

    #include <sti/engine/ShotConfig.h>
    using STI::Engine::ShotConfig;

    #include "JShot.h"
    using STI::Engine::JShot;

    #include "JChannelManager.h"
    #include <sti/fwd/Channel_fwd.h>
    #include <sti/device/Channel.h>
    #include <sti/device/LocalChannel.h>
    using STI::Device::LocalChannel;

    #include "ChannelRefreshListener.h"
    using STI::Device::ChannelRefreshListener;

    #include <sti/engine/ShotResult.h>
    
    #include "JEventEngine.h"

    #include <functional>

%}




//***** Shared pointer definitions ********//

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::JLocalDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);
%shared_ptr(STI::Device::JDeviceMessageReceiver);
%shared_ptr(STI::Device::JDeviceMessageDispatcher);
%shared_ptr(STI::Engine::JEventEngineScheduler);
// %shared_ptr(STI::Device::ChannelManager);
%shared_ptr(STI::Device::JChannelManager);
%shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::JPersistenceManager);

%shared_ptr(STI::Device::Channel);
%shared_ptr(STI::Device::LocalChannel);




%shared_ptr(STI::Engine::JShot);

%shared_ptr(STI::Engine::SynchronousEvent);
%shared_ptr(STI::Engine::SynchronousEventAdapter);

//%shared_ptr(STI::Device::DeviceMessageReceiver);

//Messages
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

// %shared_ptr(STI::Utils::FileHolder);

////////////////////////////////////





%include "sti/device/DeviceTrace.h"

//EngineID
%include "sti/engine/EngineID.h"


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
%include "sti/device/DeviceMessageType.h"
%include "sti/device/DeviceMessage.h"

%include "sti/device/DeviceMessageListener.h"

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
%include "sti/fwd/Channel_fwd.h"
%include "sti/device/Channel.h"
%template(ChannelVector) std::vector< std::shared_ptr < STI::Device::Channel > >;
%ignore STI::Device::ChannelManager;
%ignore STI::Device::JChannelManager::JChannelManager(std::shared_ptr< STI::Device::ChannelManager >& manager);
%include "JChannelManager.h"

%include "sti/device/LocalChannel.h"
%include "ChannelRefreshListener.h"

//EngineJobSourceID
%include "sti/engine/EngineJobSourceID.h"

//ShotConfig
%include "sti/engine/ShotConfig.h"


%include "sti/engine/TimeStamp.h"
%include "sti/engine/ParseID.h"
%include "sti/engine/ShotID.h"





//SynchronousEvent
%include "sti/engine/SynchronousEvent.h"
%template(SynchronousEventVector) std::vector< std::shared_ptr < STI::Engine::SynchronousEventAdapter > >;


%include "sti/utils/LocalCollection.h"
// %template(DeviceCollectionListener) STI::Utils::LocalCollectionListenerAdapter< STI::Device::DeviceID >;


%shared_ptr(STI::Utils::LocalCollectionListenerAdapter< STI::Device::DeviceID >);
%template(DeviceCollectionListener) STI::Utils::LocalCollectionListenerAdapter< STI::Device::DeviceID >;


// %typemap(jstype) char** "String[]"
//JLocalDevice
%include "JLocalDevice.h"

//EngineJobID
%template(EngineJobIDSet) std::set< STI::Engine::EngineJobID >;
%include "sti/engine/EngineJobID.h"


//JShot
%ignore STI::Engine::JShot::JShot(std::shared_ptr< STI::Engine::Shot >& shot);
%include "JShot.h"


%include "sti/engine/EventEngineJobList.h"
%include "sti/engine/EngineJobStatus.h"



//ChannelUpdateMessage
%template(ChannelMixedValueMap) std::map< short, STI::Utils::MixedValue >;
