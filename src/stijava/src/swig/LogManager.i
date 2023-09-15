
%{
    #include "JLogManager.h"
    using STI::Device::JLogManager;

    #include <sti/device/LogID.h>
    using STI::Device::LogID;

    #include <sti/device/LogRecord.h>
    using STI::Device::LogRecord;
    using STI::Device::DeviceLogRecord;

    #include <sti/device/LogFile.h>
    using STI::Device::LogFile;

    #include <sti/device/LogFileFilter.h>
    using STI::Device::LogFileFilter;


%}

// %shared_ptr(STI::Device::LogManager);

%template(LogIDVector) std::vector< STI::Device::LogID >;
%template(LogFileVector) std::vector< STI::Device::LogFile >;

%include "sti/device/LogID.h"


%template(DeviceLogRecordMap) std::map< std::string, STI::Device::DeviceLogRecord >;

%include "sti/device/LogRecord.h"

%include "sti/device/LogFile.h"
%include "sti/device/LogFileFilter.h"


%ignore STI::Device::LogManager;
%ignore STI::Device::JLogManager::JLogManager(const std::shared_ptr< STI::Device::LogManager >& manager);
%include "JLogManager.h"
