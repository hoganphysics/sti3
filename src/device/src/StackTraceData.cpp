#include "StackTraceData.h"

#include <sti/engine/StackTrace.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/LocalFileHolder.h>

#include "RawStackTrace.h"

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

using STI::Engine::StackTraceData;
using STI::Engine::StackTrace;
using STI::Engine::RawStackTrace;


StackTraceData::StackTraceData()
: StackTraceData(0)
{
}

StackTraceData::StackTraceData(const std::shared_ptr<STI::Utils::FileHolderFactory>& fileFactory)
: fileHolderFactory(fileFactory)
{
    init();
}

StackTraceData::StackTraceData(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& timingFiles, const std::vector<std::string>& functionNames)
: timingFiles(timingFiles), functionNames(functionNames)
{
    init();
}

void StackTraceData::init()
{
    if (fileHolderFactory == 0) {
        fileHolderFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
    }

    fileMap = std::make_shared<VectorMapFileHolder>(timingFiles);
    functionMap = std::make_shared<VectorMapString>(functionNames);
}


StackTrace StackTraceData::addStackTrace(const RawStackTrace& rawStackTrace)
{
    // std::unique_lock<std::mutex> fileLock(stackDataMutex);

    StackTrace stackTrace;
    
    for (auto& rawFrame : rawStackTrace.getFrames()) {
        stackTrace.appendFrame(
            addFile(rawFrame.file), 
            rawFrame.line,
            functionMap->add(rawFrame.func, rawFrame.func)            
            );
    }
    return stackTrace;
}

RawStackTrace StackTraceData::getStackTrace(const StackTrace& stackTrace) const
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    std::shared_ptr<STI::Utils::FileHolder> file;
    std::string func;

    RawStackTrace rawTrace;

    for (auto& frame : stackTrace.getFrames()) {

        if (fileMap->at(frame.file, file) && file != 0 && functionMap->at(frame.func, func)) {
            rawTrace.appendFrame(file->getFilename(), frame.line, func);
        }        
    }
    return rawTrace;
}


std::vector<std::shared_ptr<STI::Utils::FileHolder>> StackTraceData::getTimingFiles() const
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);
    return fileMap->getVec();
}


// std::vector<std::string> timingFileNames()
std::vector<std::string> StackTraceData::getFunctionNames() const
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    return functionMap->getVec();
}

unsigned StackTraceData::addFile(const std::string& filename)
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    unsigned index;

    if (fileMap->getIndex(filename, index)) {
        return index;
    }

    //new file
    auto file = fileHolderFactory->makeFileHolder(filename);
    return fileMap->add(filename, file);
}

template<class Archive>
void StackTraceData::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("timingFiles", timingFiles),
        cereal::make_nvp("functionNames", functionNames)
        );
}

template void StackTraceData::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void StackTraceData::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

