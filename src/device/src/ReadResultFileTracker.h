#ifndef STI_DEVICE_READRESULTFILETRACKER_H
#define STI_DEVICE_READRESULTFILETRACKER_H

#include <sti/utils/FileID.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace STI
{
namespace Utils
{
class FileServer;
}

namespace Device
{

class ReadResultFileScope;
class ReadResultFileTrackingSuspension;

class ReadResultFileTracker
{
public:
    using FileServerProvider = std::function<std::shared_ptr<STI::Utils::FileServer>()>;

    explicit ReadResultFileTracker(FileServerProvider fileServerProvider);

    void recordFile(const STI::Utils::FileID& fileID);
    void replaceFiles(short channel, const std::vector<STI::Utils::FileID>& fileIDs);
    void deleteFiles(const std::vector<STI::Utils::FileID>& fileIDs);

private:
    friend class ReadResultFileScope;
    friend class ReadResultFileTrackingSuspension;

    static thread_local std::vector<ReadResultFileScope*> activeScopes;
    static thread_local unsigned trackingSuspended;

    FileServerProvider fileServerProvider;
    std::mutex mutex;
    std::map<short, std::vector<STI::Utils::FileID>> filesByChannel;
};

class ReadResultFileScope
{
public:
    ReadResultFileScope(ReadResultFileTracker& tracker, short channel);
    ~ReadResultFileScope();

    ReadResultFileScope(const ReadResultFileScope&) = delete;
    ReadResultFileScope& operator=(const ReadResultFileScope&) = delete;
    ReadResultFileScope(ReadResultFileScope&&) = delete;
    ReadResultFileScope& operator=(ReadResultFileScope&&) = delete;

    const std::vector<STI::Utils::FileID>& files() const;
    void commit();
    void discard();

private:
    friend class ReadResultFileTracker;

    ReadResultFileTracker& tracker;
    short channel;
    std::vector<STI::Utils::FileID> fileIDs;
    bool resolved = false;
};

class ReadResultFileTrackingSuspension
{
public:
    ReadResultFileTrackingSuspension();
    ~ReadResultFileTrackingSuspension();

    ReadResultFileTrackingSuspension(const ReadResultFileTrackingSuspension&) = delete;
    ReadResultFileTrackingSuspension& operator=(const ReadResultFileTrackingSuspension&) = delete;
    ReadResultFileTrackingSuspension(ReadResultFileTrackingSuspension&&) = delete;
    ReadResultFileTrackingSuspension& operator=(ReadResultFileTrackingSuspension&&) = delete;
};

} //Device
} //STI

#endif
