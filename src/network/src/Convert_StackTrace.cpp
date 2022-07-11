
#include "Convert_StackTrace.h"

#include <sti/engine/StackTrace.h>


using STI::Network::convert;

using STI::TNetwork::TStackFrame;
using STI::Engine::StackFrame;
using STI::TNetwork::TStackFrameSeq;
using STI::Engine::StackTrace;



//StackFrame
template<>
bool STI::Network::convert<TStackFrame, StackFrame>(const TStackFrame& tStackFrame, StackFrame& stackFrame)
{
    stackFrame = convert<TStackFrame, StackFrame>(tStackFrame);
    return true;
}

template<>
bool STI::Network::convert<StackFrame, TStackFrame>(const StackFrame& stackFrame, TStackFrame& tStackFrame)
{
    tStackFrame = convert<StackFrame, TStackFrame>(stackFrame);
    return true;
}

template<>
StackFrame STI::Network::convert<TStackFrame, StackFrame>(const TStackFrame& tStackFrame)
{
    StackFrame frame;

    frame.file = static_cast<unsigned>(tStackFrame.file);
    frame.func = static_cast<unsigned>(tStackFrame.func);
    frame.line = static_cast<unsigned>(tStackFrame.line);

    return frame;
}

template<>
TStackFrame STI::Network::convert<StackFrame, TStackFrame>(const StackFrame& stackFrame)
{
    TStackFrame tframe;

    tframe.file = static_cast<CORBA::ULong>(stackFrame.file);
    tframe.func = static_cast<CORBA::ULong>(stackFrame.func);
    tframe.line = static_cast<CORBA::ULong>(stackFrame.line);
    
    return tframe;
}


//StackTrace
template<>
bool STI::Network::convert<TStackFrameSeq, StackTrace>(const TStackFrameSeq& tStackFrameSeq, StackTrace& stackFrame)
{
    for (unsigned i = 0; i < tStackFrameSeq.length(); ++i) {
        stackFrame.appendFrame( convert<TStackFrame, StackFrame>(tStackFrameSeq[i]) );
    }
    return true;
}

template<>
bool STI::Network::convert<StackTrace, TStackFrameSeq>(const StackTrace& stackFrame, TStackFrameSeq& tStackFrameSeq)
{
    convert<StackFrame, TStackFrame>(stackFrame.getFrames(), tStackFrameSeq);
    
    return true;
}


