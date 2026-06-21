#include <sti/engine/PostProcessRequest.h>

using STI::Engine::PostProcessRequest;
using STI::Engine::PostProcessTarget;
using STI::Engine::CompressedStackTrace;
using STI::Utils::MetaData;


PostProcessRequest::PostProcessRequest()
: _target(), _options(), _trace()
{
}

PostProcessRequest::PostProcessRequest(const PostProcessTarget& target, const MetaData& options)
: _target(target), _options(options), _trace()
{
}

PostProcessRequest::PostProcessRequest(const PostProcessTarget& target, const MetaData& options,
                                       const CompressedStackTrace& trace)
: _target(target), _options(options), _trace(trace)
{
}

const PostProcessTarget& PostProcessRequest::target() const
{
    return _target;
}

const MetaData& PostProcessRequest::options() const
{
    return _options;
}

const CompressedStackTrace& PostProcessRequest::trace() const
{
    return _trace;
}
