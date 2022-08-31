
#include "RawStackTrace.h"

using STI::Engine::RawStackFrame;
using STI::Engine::RawStackTrace;


//// RawStackFrame ///////

RawStackFrame::RawStackFrame()
{
}

RawStackFrame::RawStackFrame(const std::string& file, unsigned line, const std::string& func)
: file(file), line(line), func(func)
{
    
}

///////// RawStackTrace /////////

RawStackTrace::RawStackTrace()
{
}

void RawStackTrace::appendFrame(const RawStackFrame& frame)
{
    frames.push_back(frame);
}

void RawStackTrace::appendFrame(const std::string& file, unsigned line, const std::string& func)
{
    frames.push_back(RawStackFrame(file, line, func));
}

std::vector<RawStackFrame> RawStackTrace::getFrames() const
{
    return frames;
}
