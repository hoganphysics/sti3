

%{
    #include <sti/engine/Measurement.h>
    // using STI::Engine::Measurement;
    // using STI::Engine::MeasurementVector;


    #include <sti/engine/ShotResultRecord.h>
    using STI::Engine::ShotResultRecord;

    #include <sti/engine/ShotResult.h>
    using STI::Engine::ShotResult;

    #include "JPersistenceManager.h"

    // #include <sti/engine/RawEvent.h>
    // using STI::Engine::RawEvent;


    #include <sti/device/DeviceID.h>

%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"


%template(ShotResultRecordVector) std::vector< STI::Engine::ShotResultRecord >;

%shared_ptr(STI::Engine::ShotResult);
%shared_ptr(STI::Engine::Measurement);



//RawEvent
// %template(RawEventVector) std::vector< STI::Engine::RawEvent >;
//%template(RawEventMap) std::map< STI::Device::DeviceID, std::vector< STI::Engine::RawEvent > >;


//ShotResultRecord
%include "sti/engine/ShotResultRecord.h"


//Measurement -- note that ordering of the %include here is very important
%include "sti/fwd/Measurement_fwd.h"
%include "sti/engine/Measurement.h"
%template(MeasurementVector) std::vector< std::shared_ptr< STI::Engine::Measurement > >;
%shared_ptr( std::vector< std::shared_ptr< STI::Engine::Measurement > > );


//ShotResult
%template(DeviceAttributeMap) std::map< STI::Device::DeviceID, std::map< std::string, std::string > >;

%immutable STI::Engine::ShotResult::sid;
%immutable STI::Engine::ShotResult::playTime;
%immutable STI::Engine::ShotResult::parsedEvents;
%immutable STI::Engine::ShotResult::timingFiles;
%immutable STI::Engine::ShotResult::measurements;
%immutable STI::Engine::ShotResult::attributes;
%immutable STI::Engine::ShotResult::shotResultRecord;

%include "sti/engine/ShotResult.h"

//JPersistenceManager
%ignore STI::Device::JPersistenceManager::JPersistenceManager(std::shared_ptr< STI::Device::PersistenceManager >& manager);
%include "JPersistenceManager.h"

