#include "StackTrace.h"

using STI::Engine::StackFrame;
using STI::Engine::StackTrace;


//// StackFrame ///////

StackFrame::StackFrame()
{
}

StackFrame::StackFrame(const std::string& file, unsigned line, const std::string& func)
: file(file), line(line), func(func)
{
    
}

///////// StackTrace /////////

StackTrace::StackTrace()
{
}

void StackTrace::appendFrame(const StackFrame& frame)
{
    frames.push_back(frame);
}

void StackTrace::appendFrame(const std::string& file, unsigned line, const std::string& func)
{
    frames.push_back(StackFrame(file, line, func));
}

std::vector<StackFrame> StackTrace::getFrames() const
{
    return frames;
}

std::string StackTrace::print() const
{
    std::string result;
    for (const auto& frame : frames) {
        result += frame.file + ":" + std::to_string(frame.line) + " in " + frame.func + "\n";
    }
    return result;
}