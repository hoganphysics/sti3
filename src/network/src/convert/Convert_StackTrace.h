
#ifndef STI_NETWORK_CONVERT_STACKTRACE_H
#define STI_NETWORK_CONVERT_STACKTRACE_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class CompressedStackTrace;
class CompressedStackFrame;
class StackTraceResult;
class StackTraceData;

} //Engine




//CompressedStackFrame
template<>
Engine::CompressedStackFrame Network::convert<TNetwork::TStackFrame, Engine::CompressedStackFrame>(
    const TNetwork::TStackFrame& tStackFrame);
template<>
TNetwork::TStackFrame Network::convert<Engine::CompressedStackFrame, TNetwork::TStackFrame>(
    const Engine::CompressedStackFrame& stackFrame);

template<>
bool Network::convert<TNetwork::TStackFrame, Engine::CompressedStackFrame>(
        const TNetwork::TStackFrame& tStackFrame, Engine::CompressedStackFrame& stackFrame);
template<>
bool Network::convert<Engine::CompressedStackFrame, TNetwork::TStackFrame>(
        const Engine::CompressedStackFrame& stackFrame, TNetwork::TStackFrame& tStackFrame);



//CompressedStackTrace
template<>
bool Network::convert<TNetwork::TStackFrameSeq, Engine::CompressedStackTrace>(
    const TNetwork::TStackFrameSeq& tStackFrameSeq, Engine::CompressedStackTrace& stackFrame);
template<>
bool Network::convert<Engine::CompressedStackTrace, TNetwork::TStackFrameSeq>(
    const Engine::CompressedStackTrace& stackFrame, TNetwork::TStackFrameSeq& tStackFrameSeq);


//StackTraceResult
template<>
bool Network::convert<TNetwork::TStackTraceResult, std::shared_ptr<Engine::StackTraceResult>>(
    const TNetwork::TStackTraceResult& tStackTraceResult, std::shared_ptr<Engine::StackTraceResult>& stackTraceResult);
template<>
bool Network::convert<std::shared_ptr<Engine::StackTraceResult>, TNetwork::TStackTraceResult>(
    const std::shared_ptr<Engine::StackTraceResult>& stackTraceResult, TNetwork::TStackTraceResult& tStackTraceResult);


//StackTraceData
template<>
bool Network::convert<TNetwork::TStackTraceData, std::shared_ptr<Engine::StackTraceData>>(
    const TNetwork::TStackTraceData& tStackTraceData, std::shared_ptr<Engine::StackTraceData>& stackTraceData);
template<>
bool Network::convert<std::shared_ptr<Engine::StackTraceData>, TNetwork::TStackTraceData>(
    const std::shared_ptr<Engine::StackTraceData>& stackTraceData, TNetwork::TStackTraceData& tStackTraceData);




} //STI

#endif

