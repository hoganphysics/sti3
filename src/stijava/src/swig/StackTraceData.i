
%{
    #include <sti/engine/StackTraceData.h>
    using STI::Engine::StackTraceData;

    #include <sti/engine/StackTrace.h>
    using STI::Engine::StackTrace;
    #include "RawStackTrace.h"
    using STI::Engine::RawStackTrace;
    using STI::Engine::RawStackFrame;

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


//StackTrace
%template(StackFrameVector) std::vector< STI::Engine::StackFrame >;
%template(RawStackFrameVector) std::vector< STI::Engine::RawStackFrame >;
%include "sti/engine/StackTrace.h"
%include "RawStackTrace.h"

%ignore STI::Engine::StackTraceData::StackTraceData(const std::shared_ptr< STI::Utils::FileHolderFactory >& fileFactory);

%ignore STI::Engine::StackTraceData::setFileHolderFactory(const std::shared_ptr< STI::Utils::FileHolderFactory >& fileFactory);

%include <sti/engine/StackTraceData.h>
