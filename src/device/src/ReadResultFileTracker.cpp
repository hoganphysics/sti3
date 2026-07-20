#include "ReadResultFileTracker.h"

#include <sti/utils/FileServer.h>

#include <set>
#include <utility>

namespace STI
{
namespace Device
{

thread_local std::vector<ReadResultFileScope*> ReadResultFileTracker::activeScopes;
thread_local unsigned ReadResultFileTracker::trackingSuspended = 0;

ReadResultFileTracker::ReadResultFileTracker(FileServerProvider fileServerProvider)
    : fileServerProvider(std::move(fileServerProvider))
{
}

void ReadResultFileTracker::recordFile(const STI::Utils::FileID& fileID)
{
    if (trackingSuspended > 0 || activeScopes.empty()) {
        return;
    }

    auto* scope = activeScopes.back();
    if (scope != nullptr && &scope->tracker == this && !fileID.filename.empty()) {
        scope->fileIDs.push_back(fileID);
    }
}

void ReadResultFileTracker::replaceFiles(short channel, const std::vector<STI::Utils::FileID>& fileIDs)
{
    std::vector<STI::Utils::FileID> previousFileIDs;
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = filesByChannel.find(channel);
        if (it != filesByChannel.end()) {
            previousFileIDs = it->second;
        }

        if (fileIDs.empty()) {
            filesByChannel.erase(channel);
        }
        else {
            filesByChannel[channel] = fileIDs;
        }
    }

    std::set<STI::Utils::FileID> currentFileIDs(fileIDs.begin(), fileIDs.end());
    std::vector<STI::Utils::FileID> filesToDelete;
    for (const auto& previousFileID : previousFileIDs) {
        if (currentFileIDs.find(previousFileID) == currentFileIDs.end()) {
            filesToDelete.push_back(previousFileID);
        }
    }
    deleteFiles(filesToDelete);
}

void ReadResultFileTracker::deleteFiles(const std::vector<STI::Utils::FileID>& fileIDs)
{
    if (fileIDs.empty() || fileServerProvider == nullptr) {
        return;
    }

    auto fileServer = fileServerProvider();
    if (fileServer == nullptr) {
        return;
    }

    for (const auto& fileID : fileIDs) {
        if (!fileID.filename.empty()) {
            fileServer->deleteFile(fileID);
        }
    }
}

ReadResultFileScope::ReadResultFileScope(ReadResultFileTracker& tracker, short channel)
    : tracker(tracker), channel(channel)
{
    ReadResultFileTracker::activeScopes.push_back(this);
}

ReadResultFileScope::~ReadResultFileScope()
{
    if (!ReadResultFileTracker::activeScopes.empty() && ReadResultFileTracker::activeScopes.back() == this) {
        ReadResultFileTracker::activeScopes.pop_back();
    }

    if (!resolved) {
        discard();
    }
}

const std::vector<STI::Utils::FileID>& ReadResultFileScope::files() const
{
    return fileIDs;
}

void ReadResultFileScope::commit()
{
    if (!resolved) {
        tracker.replaceFiles(channel, fileIDs);
        resolved = true;
    }
}

void ReadResultFileScope::discard()
{
    if (!resolved) {
        tracker.deleteFiles(fileIDs);
        resolved = true;
    }
}

ReadResultFileTrackingSuspension::ReadResultFileTrackingSuspension()
{
    ReadResultFileTracker::trackingSuspended++;
}

ReadResultFileTrackingSuspension::~ReadResultFileTrackingSuspension()
{
    if (ReadResultFileTracker::trackingSuspended > 0) {
        ReadResultFileTracker::trackingSuspended--;
    }
}

} //Device
} //STI
