#ifndef STI_DEVICE_LOCALPOSTPROCESSINGMANAGER_H
#define STI_DEVICE_LOCALPOSTPROCESSINGMANAGER_H

#include <sti/device/PostProcessingManager.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceCollection.h>
#include <sti/engine/ShotID.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/EventQueue.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


namespace STI
{
namespace Device
{

class DeviceMessageDispatcher;
class PersistenceManager;
class Logger;

enum class PostProcessingStatus;   //defined in DeviceMessage.h


//One work item per requestPostProcessing() call. Default-constructible as
//required by EventQueue<Event>.
struct PostProcessWorkItem
{
    std::string targetName;
    STI::Engine::ShotID shotID;
    STI::Device::DeviceID shotOwnerID;
    STI::Utils::MetaData options;
};


//Concrete PostProcessingManager backed by a single EventQueue worker thread per
//device (one shared worker, not per target — see docs/notes/postProcess.md,
//decision 6). The worker thread is started lazily on the first registered
//target. Device lookup is via the abstract DeviceCollection so the manager
//never references stinetwork (AGENTS.md dependency boundary).
class LocalPostProcessingManager : public PostProcessingManager,
                                   private STI::Utils::EventQueue<PostProcessWorkItem>
{
public:

    LocalPostProcessingManager(const STI::Device::DeviceID& deviceID,
                               const std::shared_ptr<DeviceMessageDispatcher>& dispatcher,
                               const std::shared_ptr<DeviceCollection>& deviceCollection,
                               const std::shared_ptr<PersistenceManager>& persistenceManager,
                               Logger* logger = nullptr);

    ~LocalPostProcessingManager() override;

    void addPostProcessingTarget(const std::string& name,
                                 PostProcessingFunction function,
                                 const std::string& description = "") override;

    std::vector<PostProcessingTargetInfo> getPostProcessingTargets() const override;

    bool requestPostProcessing(const std::string& name,
                               const STI::Engine::ShotID& shotID,
                               const STI::Device::DeviceID& shotOwnerID,
                               const STI::Utils::MetaData& options) override;

    void stop() override;

private:

    //EventQueue worker entry point.
    void handleEvent(const PostProcessWorkItem& item) override;

    bool getTarget(const std::string& name, PostProcessingFunction& function) const;

    //Best-effort: ensure the owning device's results are persisted before running
    //the callback. Logs (does not throw) if unavailable.
    void ensureResultsAvailable(const STI::Engine::ShotID& shotID,
                                const STI::Device::DeviceID& shotOwnerID);

    void dispatchComplete(const PostProcessWorkItem& item,
                          PostProcessingStatus status,
                          const STI::Utils::MetaData& results,
                          const std::string& errorMessage);

    STI::Device::DeviceID deviceID;
    std::shared_ptr<DeviceMessageDispatcher> dispatcher;
    std::shared_ptr<DeviceCollection> deviceCollection;
    std::shared_ptr<PersistenceManager> persistenceManager;
    Logger* logger;

    mutable std::mutex registryMutex;
    std::map<std::string, PostProcessingFunction> targets;
    std::map<std::string, std::string> descriptions;
    bool workerStarted;
};


} //Device
} //STI

#endif
