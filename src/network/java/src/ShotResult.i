

%{
    #include "Measurement.h"
    // using STI::Engine::Measurement;
    // using STI::Engine::MeasurementVector;


    #include "ShotResultRecord.h"
    using STI::Engine::ShotResultRecord;

    #include "ShotResult.h"
    using STI::Engine::ShotResult;

    #include "JPersistenceManager.h"

    #include "RawEvent.h"
    using STI::Engine::RawEvent;


    #include "DeviceID.h"

    #include "FileHolder.h"
    using STI::Utils::FileHolder;
%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"


%template(ShotResultRecordVector) std::vector< STI::Engine::ShotResultRecord >;

%shared_ptr(STI::Engine::ShotResult);
%shared_ptr(STI::Engine::Measurement);

%shared_ptr(STI::Utils::FileHolder);

//FileHolder
%template(FileHolderVector) std::vector< std::shared_ptr< STI::Utils::FileHolder > >;
%include "FileHolder.h"


//RawEvent
// %template(RawEventVector) std::vector< STI::Engine::RawEvent >;
%template(RawEventMap) std::map< STI::Device::DeviceID, std::vector< STI::Engine::RawEvent > >;


//ShotResultRecord
%include "ShotResultRecord.h"


//Measurement -- note that ordering of the %include here is very important
%include "fwd/Measurement_fwd.h"
%include "Measurement.h"
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

%include "ShotResult.h"

//JPersistenceManager
%ignore STI::Device::JPersistenceManager::JPersistenceManager(std::shared_ptr< STI::Device::PersistenceManager >& manager);
%include "JPersistenceManager.h"

