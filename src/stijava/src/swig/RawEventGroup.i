
%{
    #include <sti/engine/RawEventGroup.h>
    using STI::Engine::RawEventGroup;

    #include <sti/engine/ParsedTag.h>
    using STI::Engine::ParsedTag;
    #include <sti/engine/ParsedVar.h>
    using STI::Engine::ParsedVar;

    #include <sti/engine/RawEventTargetChannel.h>
    using STI::Engine::RawEventTargetChannel;
    #include <sti/engine/RawEventTargetDevice.h>
    using STI::Engine::RawEventTargetDevice;
    #include <sti/engine/RawEventTarget.h>
    using STI::Engine::RawEventTarget;
    #include <sti/engine/RawEvent.h>
    using STI::Engine::RawEventType;

%}

%include "std_shared_ptr.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_set.i"


%shared_ptr(STI::Engine::RawEventGroup);

%template(RawEventGroupVector) std::vector< std::shared_ptr< STI::Engine::RawEventGroup > >;


//RawEvent
%include "sti/engine/RawEventTargetChannel.h"
%include "sti/engine/RawEventTargetDevice.h"
%include "sti/engine/RawEventTarget.h"
%include "sti/fwd/RawEvent_fwd.h"
%include "sti/engine/RawEvent.h"
%template(RawEventVector) std::vector< STI::Engine::RawEvent >;
%shared_ptr( std::vector< STI::Engine::RawEvent > );

%template(ParsedTagVector) std::vector< STI::Engine::ParsedTag >;
%template(ParsedVarVector) std::vector< STI::Engine::ParsedVar >;
%template(ParsedVarSet) std::set< STI::Engine::ParsedVar >;

%include "sti/engine/ParsedTag.h"
%include "sti/engine/ParsedVar.h"

%template(StringDoubleMap) std::map< std::string, double >;

%template(RawEventTargetMap) std::map< std::string, STI::Engine::RawEventTarget >;
%template(RawEventTargetDeviceMap) std::map< std::string, STI::Engine::RawEventTargetDevice >; 

%ignore STI::Engine::RawEventGroup::getReferencePoint(const std::string& refName, double& time) const;
%include "sti/engine/RawEventGroup.h"

%extend STI::Engine::RawEventGroup 
{
    double STI::Engine::RawEventGroup::getReferencePoint(const std::string& refName) const
    {
        double refTime = 0;
        self->getReferencePoint(refName, refTime);
        return refTime;
    }
} 
