#ifndef STI_ENGINE_STACKTRACEDATA_H
#define STI_ENGINE_STACKTRACEDATA_H

#include <sti/utils/FileHolder.h>
#include <sti/utils/FileHolderFactory.h>
#include <sti/utils/VectorMap.h>

#include <string>
#include <vector>
#include <memory>
#include <mutex>


namespace STI
{
namespace Engine
{

class StackTrace;
class RawStackTrace;


class StackTraceData
{
public:

    StackTraceData();
    StackTraceData(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& timingFiles, const std::vector<std::string>& functionNames);
    StackTraceData(const std::shared_ptr<STI::Utils::FileHolderFactory>& fileFactory);
    
    StackTrace addStackTrace(const RawStackTrace& stackTrace);
    RawStackTrace getStackTrace(const StackTrace& stackTrace) const;

    std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const;
    // std::vector<std::string> timingFileNames()
    std::vector<std::string> getFunctionNames() const;

    void setFileHolderFactory(const std::shared_ptr<STI::Utils::FileHolderFactory>& fileFactory);

    void replaceFile(const std::string& oldFilename, const std::shared_ptr<STI::Utils::FileHolder>& newFile);
    void deleteFiles();

//    template<class Archive>
//	void serialize(Archive& archive);

    template<class Archive>
    void save(Archive& archive) const;

    template<class Archive>
    void load(Archive& archive);

private:
    
    void init();
    unsigned addFile(const std::string& filename);

    std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
    std::vector<std::string> timingFileNames;
    std::vector<std::string> functionNames;

    typedef STI::Utils::VectorMap<std::string, std::string> VectorMapString;
    std::shared_ptr<VectorMapString> functionMap;

    typedef STI::Utils::VectorMap<std::string, std::shared_ptr<STI::Utils::FileHolder>> VectorMapFileHolder;
    std::shared_ptr<VectorMapFileHolder> fileMap;

    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;

    mutable std::mutex stackDataMutex;
};


} //Engine
} //STI

#endif
