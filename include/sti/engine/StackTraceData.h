#ifndef STI_ENGINE_STACKTRACEDATA_H
#define STI_ENGINE_STACKTRACEDATA_H

#include <sti/utils/FileID.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/VectorMap.h>
#include <sti/device/DeviceID.h>

#include <string>
#include <vector>
#include <memory>
#include <mutex>


namespace STI
{
namespace Engine
{

class CompressedStackTrace;
class StackTrace;


class StackTraceData
{
public:

    StackTraceData();
    StackTraceData(const STI::Device::DeviceID& localID, const std::shared_ptr<STI::Utils::FileServer>& fileServer);
    StackTraceData(const std::vector<STI::Utils::FileID>& timingFiles, const std::vector<std::string>& functionNames);
    
    CompressedStackTrace addStackTrace(const StackTrace& stackTrace);
    StackTrace getStackTrace(const CompressedStackTrace& stackTrace) const;

    const std::vector<STI::Utils::FileID>& getTimingFiles() const;
    const std::vector<std::string>& getFunctionNames() const;

    void setFileServer(const std::shared_ptr<STI::Utils::FileServer>& server);
    bool getFileServer(std::shared_ptr<STI::Utils::FileServer>& server);

    void replaceFile(const std::string& oldFilename, const STI::Utils::FileID& newFile);

   template<class Archive>
	void serialize(Archive& archive);

private:
    
    void init();
    unsigned addFile(const std::string& filename);

    std::vector<STI::Utils::FileID> timingFiles;
    std::vector<std::string> timingFileNames;
    std::vector<std::string> functionNames;

    typedef STI::Utils::VectorMap<std::string, std::string> VectorMapString;
    std::shared_ptr<VectorMapString> functionMap;

    typedef STI::Utils::VectorMap<std::string, STI::Utils::FileID> VectorMapFileID;
    std::shared_ptr<VectorMapFileID> fileMap;

    std::shared_ptr<STI::Utils::FileServer> fileServer;

    STI::Device::DeviceID localID;

    mutable std::mutex stackDataMutex;
};


} //Engine
} //STI

#endif
