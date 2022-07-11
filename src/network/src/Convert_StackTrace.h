
#ifndef STI_NETWORK_CONVERT_STACKTRACE_H
#define STI_NETWORK_CONVERT_STACKTRACE_H

#include "NetworkConvert.h"
#include "deviceNet.h"
#include "orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class StackTrace;
class StackFrame;

} //Engine




//StackFrame
template<>
bool Network::convert<TNetwork::TStackFrame, Engine::StackFrame>(
        const TNetwork::TStackFrame& tStackFrame, Engine::StackFrame& stackFrame);
template<>
bool Network::convert<Engine::StackFrame, TNetwork::TStackFrame>(
        const Engine::StackFrame& stackFrame, TNetwork::TStackFrame& tStackFrame);
template<>
Engine::StackFrame Network::convert<TNetwork::TStackFrame, Engine::StackFrame>(
        const TNetwork::TStackFrame& tStackFrame);
template<>
TNetwork::TStackFrame Network::convert<Engine::StackFrame, TNetwork::TStackFrame>(
        const Engine::StackFrame& stackFrame);


//StackTrace
template<>
bool Network::convert<TNetwork::TStackFrameSeq, Engine::StackTrace>(
    const TNetwork::TStackFrameSeq& tStackFrameSeq, Engine::StackTrace& stackFrame);
template<>
bool Network::convert<Engine::StackTrace, TNetwork::TStackFrameSeq>(
    const Engine::StackTrace& stackFrame, TNetwork::TStackFrameSeq& tStackFrameSeq);



} //STI

#endif

