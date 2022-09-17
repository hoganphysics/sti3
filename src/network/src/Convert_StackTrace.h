
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
class StackTraceResult;
class StackTraceData;

} //Engine




//StackFrame
template<>
Engine::StackFrame Network::convert<TNetwork::TStackFrame, Engine::StackFrame>(
    const TNetwork::TStackFrame& tStackFrame);
//template<>
//TNetwork::TStackFrame Network::convert<Engine::StackFrame, TNetwork::TStackFrame>(
//    const Engine::StackFrame& stackFrame);

template<>
bool Network::convert<TNetwork::TStackFrame, Engine::StackFrame>(
        const TNetwork::TStackFrame& tStackFrame, Engine::StackFrame& stackFrame);
template<>
bool Network::convert<Engine::StackFrame, TNetwork::TStackFrame>(
        const Engine::StackFrame& stackFrame, TNetwork::TStackFrame& tStackFrame);



//StackTrace
template<>
bool Network::convert<TNetwork::TStackFrameSeq, Engine::StackTrace>(
    const TNetwork::TStackFrameSeq& tStackFrameSeq, Engine::StackTrace& stackFrame);
template<>
bool Network::convert<Engine::StackTrace, TNetwork::TStackFrameSeq>(
    const Engine::StackTrace& stackFrame, TNetwork::TStackFrameSeq& tStackFrameSeq);


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

