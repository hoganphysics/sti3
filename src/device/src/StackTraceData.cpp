#include <sti/engine/StackTraceData.h>

#include <sti/engine/StackTrace.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileID.h>

#include "RawStackTrace.h"

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <filesystem>
namespace fs = std::filesystem;

using STI::Engine::StackTraceData;
using STI::Engine::StackTrace;
using STI::Engine::RawStackTrace;


StackTraceData::StackTraceData()
{
    init();
}

StackTraceData::StackTraceData(const STI::Device::DeviceID& localID, const std::shared_ptr<STI::Utils::FileServer>& fileServer)
: localID(localID), fileServer(fileServer)

{
    init();
}

StackTraceData::StackTraceData(const std::vector<STI::Utils::FileID>& files, const std::vector<std::string>& funcNames)
{
    init();

    for (auto& fileID : files) {
        fileMap->add(fileID.getFullFilename(), fileID);
        timingFileNames.push_back(fileID.getFullFilename());
    }

    for (auto& name : funcNames) {
        functionMap->add(name, name);
    }
}

void StackTraceData::init()
{
    fileMap = std::make_shared<VectorMapFileID>(timingFiles);
    functionMap = std::make_shared<VectorMapString>(functionNames);
}


StackTrace StackTraceData::addStackTrace(const RawStackTrace& rawStackTrace)
{
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

    STI::Utils::FileID fileID;
    std::string func;

    RawStackTrace rawTrace;

    for (auto& frame : stackTrace.getFrames()) {

        if (fileMap->at(frame.file, fileID) && functionMap->at(frame.func, func)) {
            rawTrace.appendFrame(fileID.getFullFilename(), frame.line, func);
        }        
    }
    return rawTrace;
}


const std::vector<STI::Utils::FileID>& StackTraceData::getTimingFiles() const
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);
    return fileMap->vec();
}


const std::vector<std::string>& StackTraceData::getFunctionNames() const
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    return functionMap->vec();
}

unsigned StackTraceData::addFile(const std::string& filename)
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    unsigned index;

    if (fileMap->getIndex(filename, index)) {
        return index;
    }

    //new file
    STI::Utils::FileID fileID;
    fs::path filepath = filename;
    
    fileID.filename = filepath.filename();
    fileID.path = filepath.parent_path();
    fileID.origin = localID.getID();
    fileID.persistenceLocation = localID.getID();

    timingFileNames.push_back(filename);
    return fileMap->add(filename, fileID);
}

void StackTraceData::replaceFile(const std::string& oldFilename, const STI::Utils::FileID& newFile)
{
    std::unique_lock<std::mutex> fileLock(stackDataMutex);

    fileMap->rename(oldFilename, newFile.getFullFilename());
    fileMap->replace(newFile.getFullFilename(), newFile);

    auto it = std::find(timingFileNames.begin(), timingFileNames.end(), oldFilename);
    if (it != timingFileNames.end()) {
        timingFileNames.erase(it);
    }
    timingFileNames.push_back(newFile.getFullFilename());
}


bool StackTraceData::getFileServer(std::shared_ptr<STI::Utils::FileServer>& server)
{
    server = fileServer;
    return (server != 0);
}

void StackTraceData::setFileServer(const std::shared_ptr<STI::Utils::FileServer>& server)
{
    fileServer = server;
}

template<class Archive>
void StackTraceData::serialize(Archive& archive)
{
   archive( 
       cereal::make_nvp("localID", localID),
       cereal::make_nvp("timingFiles", timingFiles),
       cereal::make_nvp("functionNames", functionNames)
       );
}

template void StackTraceData::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void StackTraceData::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

