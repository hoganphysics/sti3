#include "Convert_StackTrace.h"
#include "Convert_EventEngine.h"
#include "Convert_ResultsCollector.h"
#include "Convert_File.h"
#include "TFileServerRefInterface.h"

#include <sti/engine/CompressedStackTrace.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/StackTraceResult.h>

using STI::Network::convert;
using STI::TNetwork::TStackFrame;
using STI::Engine::CompressedStackFrame;
using STI::TNetwork::TStackFrameSeq;
using STI::Engine::CompressedStackTrace;
using STI::TNetwork::TStackTraceResult;
using STI::Engine::StackTraceResult;
using STI::TNetwork::TParseID;
using STI::Engine::ParseID;
using STI::TNetwork::TStackTraceData;
using STI::Engine::StackTraceData;


//CompressedStackFrame
template<>
bool STI::Network::convert<TStackFrame, CompressedStackFrame>(const TStackFrame& tStackFrame, CompressedStackFrame& stackFrame)
{
    stackFrame = convert<TStackFrame, CompressedStackFrame>(tStackFrame);
    return true;
}

template<>
bool STI::Network::convert<CompressedStackFrame, TStackFrame>(const CompressedStackFrame& stackFrame, TStackFrame& tStackFrame)
{
    tStackFrame = convert<CompressedStackFrame, TStackFrame>(stackFrame);
    return true;
}

template<>
CompressedStackFrame STI::Network::convert<TStackFrame, CompressedStackFrame>(const TStackFrame& tStackFrame)
{
    CompressedStackFrame frame;

    frame.file = static_cast<unsigned>(tStackFrame.file);
    frame.func = static_cast<unsigned>(tStackFrame.func);
    frame.line = static_cast<unsigned>(tStackFrame.line);

    return frame;
}

template<>
TStackFrame STI::Network::convert<CompressedStackFrame, TStackFrame>(const CompressedStackFrame& stackFrame)
{
    TStackFrame tframe;

    tframe.file = static_cast<CORBA::ULong>(stackFrame.file);
    tframe.func = static_cast<CORBA::ULong>(stackFrame.func);
    tframe.line = static_cast<CORBA::ULong>(stackFrame.line);
    
    return tframe;
}


//CompressedStackTrace
template<>
bool STI::Network::convert<TStackFrameSeq, CompressedStackTrace>(const TStackFrameSeq& tStackFrameSeq, CompressedStackTrace& stackFrame)
{
    for (unsigned i = 0; i < tStackFrameSeq.length(); ++i) {
        stackFrame.appendFrame( convert<TStackFrame, CompressedStackFrame>(tStackFrameSeq[i]) );
    }
    return true;
}

template<>
bool STI::Network::convert<CompressedStackTrace, TStackFrameSeq>(const CompressedStackTrace& stackFrame, TStackFrameSeq& tStackFrameSeq)
{
    return convert<CompressedStackFrame, TStackFrame>(stackFrame.getFrames(), tStackFrameSeq);
}


//StackTraceResult
template<>
bool STI::Network::convert<TStackTraceResult, std::shared_ptr<StackTraceResult>>(
    const TStackTraceResult& tStackTraceResult, std::shared_ptr<StackTraceResult>& stackTraceResult)
{
    stackTraceResult = std::make_shared<StackTraceResult>();

    convert<TParseID, ParseID>(tStackTraceResult.parseID, stackTraceResult->pid);
    convert<TStackTraceData, std::shared_ptr<StackTraceData>>(
        tStackTraceResult.stackTraceData, stackTraceResult->stackTraceData);
    
    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<StackTraceResult>, TStackTraceResult>(
    const std::shared_ptr<StackTraceResult>& stackTraceResult, TStackTraceResult& tStackTraceResult)
{
    if (stackTraceResult == 0) return false;
    
    convert<ParseID, TParseID>(stackTraceResult->pid, tStackTraceResult.parseID);
    convert<std::shared_ptr<StackTraceData>, TStackTraceData>(
        stackTraceResult->stackTraceData, tStackTraceResult.stackTraceData);
    
    return true;
}


//StackTraceData
template<>
bool STI::Network::convert<TStackTraceData, std::shared_ptr<StackTraceData>>(
    const TStackTraceData& tStackTraceData, std::shared_ptr<StackTraceData>& stackTraceData)
{
    std::vector<STI::Utils::FileID> timingFiles;
    std::vector<std::string> functionNames;
    
    convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tStackTraceData.functionNames, functionNames);
    // convert<STI::TNetwork::TFileHolderSeq, std::vector<std::shared_ptr<STI::Utils::FileHolder>>>(tStackTraceData.timingFiles, timingFiles);
    convert<STI::TNetwork::TFileID, STI::Utils::FileID>(tStackTraceData.timingFiles, timingFiles);

    stackTraceData = std::make_shared<StackTraceData>(timingFiles, functionNames);

    std::shared_ptr<STI::Utils::FileServer> fileServer;
    convert<STI::TNetwork::TFileServer_var, std::shared_ptr<STI::Utils::FileServer>>(tStackTraceData.fileServer, fileServer);
    stackTraceData->setFileServer(fileServer);

    return (stackTraceData != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<StackTraceData>, TStackTraceData>(
    const std::shared_ptr<StackTraceData>& stackTraceData, TStackTraceData& tStackTraceData)
{
    if (stackTraceData == 0) return false;

    convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(
        stackTraceData->getFunctionNames(), tStackTraceData.functionNames);

    convert<STI::Utils::FileID, STI::TNetwork::TFileID>(stackTraceData->getTimingFiles(), tStackTraceData.timingFiles);

    STI::TNetwork::TFileServer_var tFileServer;

    std::shared_ptr<STI::Utils::FileServer> fileServer;
    if (stackTraceData->getFileServer(fileServer) 
            && STI::Network::TFileServerRefInterface::getTFileServerReference(fileServer, tFileServer)) {
        tStackTraceData.fileServer = tFileServer;
    }

    // convert<std::vector<std::shared_ptr<STI::Utils::FileHolder>>, STI::TNetwork::TFileHolderSeq>(
    //     stackTraceData->getTimingFiles(), tStackTraceData.timingFiles);

    return true;
}

