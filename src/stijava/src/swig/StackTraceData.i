
%{
    #include <sti/engine/StackTraceData.h>
    using STI::Engine::StackTraceData;

    #include <sti/engine/CompressedStackTrace.h>
    using STI::Engine::CompressedStackTrace;
    #include "StackTrace.h"
    using STI::Engine::StackTrace;
    using STI::Engine::StackFrame;

    // #include <sti/utils/FileHolder.h>
    // using STI::Utils::FileHolder;
%}

%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_vector.i"


// %shared_ptr(STI::Utils::FileHolder);
%shared_ptr(STI::Engine::StackTraceData);

// //FileHolder
// %template(FileHolderVector) std::vector< std::shared_ptr< STI::Utils::FileHolder > >;
// %include "sti/utils/FileHolder.h"


//CompressedStackTrace
%template(CompressedStackFrameVector) std::vector< STI::Engine::CompressedStackFrame >;
%template(StackFrameVector) std::vector< STI::Engine::StackFrame >;
%include "sti/engine/CompressedStackTrace.h"
%include "StackTrace.h"

%ignore STI::Engine::StackTraceData::StackTraceData(const std::shared_ptr< STI::Utils::FileHolderFactory >& fileFactory);
// %ignore STI::Engine::StackTraceData::StackTraceData(const STI::Device::DeviceID& localID, const std::shared_ptr< STI::Utils::FileServer >& fileServer);

%ignore STI::Engine::StackTraceData::setFileHolderFactory(const std::shared_ptr< STI::Utils::FileHolderFactory >& fileFactory);

%include <sti/engine/StackTraceData.h>
