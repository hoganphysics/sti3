
#include "StackTracePy.h"

using STI::Python::StackTracePy;
using STI::Python::StackFramePy;


//// StackFramePy ///////

StackFramePy::StackFramePy()
{
}

StackFramePy::StackFramePy(const std::string& file, unsigned line, const std::string& func)
: file(file), line(line), func(func)
{
    
}

///////// StackTracePy /////////

StackTracePy::StackTracePy()
{
}

void StackTracePy::appendFrame(const StackFramePy& frame)
{
    frames.push_back(frame);
}

void StackTracePy::appendFrame(const std::string& file, unsigned line, const std::string& func)
{
    frames.push_back(StackFramePy(file, line, func));
}

std::vector<StackFramePy> StackTracePy::getFrames() const
{
    return frames;
}

