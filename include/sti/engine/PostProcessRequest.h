#ifndef STI_ENGINE_POSTPROCESSREQUEST_H
#define STI_ENGINE_POSTPROCESSREQUEST_H


#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/CompressedStackTrace.h>
#include <sti/utils/MetaData.h>


namespace STI
{
namespace Engine
{


//Pairs a post-processing target with the per-call options supplied at the
//postProcess() call site. Collected by RawEventGroup into a side-list that
//never enters the hard-timed event table (see docs/notes/postProcess.md).
//
//Like RawEventGroup's own metaData member, the MetaData payload is not cereal-
//serialized; the request travels to the parsing server over CORBA via
//Convert_RawEventGroup (Phase 1.5).
class PostProcessRequest
{
public:

    PostProcessRequest();
    PostProcessRequest(const PostProcessTarget& target, const STI::Utils::MetaData& options);
    PostProcessRequest(const PostProcessTarget& target, const STI::Utils::MetaData& options,
                       const STI::Engine::CompressedStackTrace& trace);

    const PostProcessTarget& target() const;
    const STI::Utils::MetaData& options() const;
    const STI::Engine::CompressedStackTrace& trace() const;

private:

    PostProcessTarget _target;
    STI::Utils::MetaData _options;
    STI::Engine::CompressedStackTrace _trace;   //source location of the postProcess() call, for diagnostics
};


} //Engine
} //STI

#endif
