#include "LocalPostProcessingManager.h"

#include <sti/device/Device.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceTrace.h>
#include <sti/device/PersistenceManager.h>
#include <sti/device/Logger.h>
#include <sti/engine/ShotResult.h>

#include <exception>

using STI::Device::LocalPostProcessingManager;
using STI::Device::PostProcessWorkItem;
using STI::Device::PostProcessingFunction;
using STI::Device::PostProcessingTargetInfo;
using STI::Device::PostProcessingStatus;
using STI::Device::PostProcessingCompleteMessage;
using STI::Device::DeviceID;
using STI::Device::DeviceCollection;
using STI::Device::Device;
using STI::Device::PersistenceManager;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::Logger;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Utils::MetaData;


LocalPostProcessingManager::LocalPostProcessingManager(const DeviceID& deviceID,
                                                       const std::shared_ptr<DeviceMessageDispatcher>& dispatcher,
                                                       const std::shared_ptr<DeviceCollection>& deviceCollection,
                                                       const std::shared_ptr<PersistenceManager>& persistenceManager,
                                                       Logger* logger)
: deviceID(deviceID), dispatcher(dispatcher), deviceCollection(deviceCollection),
  persistenceManager(persistenceManager), logger(logger), workerStarted(false)
{
}

LocalPostProcessingManager::~LocalPostProcessingManager()
{
    //EventQueue::~EventQueue() also calls stop(); make shutdown explicit/idempotent.
    stop();
}

void LocalPostProcessingManager::addPostProcessingTarget(const std::string& name,
                                                         PostProcessingFunction function,
                                                         const std::string& description)
{
    {
        std::unique_lock<std::mutex> lock(registryMutex);
        targets[name] = std::move(function);
        descriptions[name] = description;
    }

    //Lazily start the worker on the first registered target so devices that never
    //post-process don't carry an idle thread. EventQueue::start() is idempotent.
    bool startWorker = false;
    {
        std::unique_lock<std::mutex> lock(registryMutex);
        if (!workerStarted) {
            workerStarted = true;
            startWorker = true;
        }
    }
    if (startWorker) {
        start();
    }
}

std::vector<PostProcessingTargetInfo> LocalPostProcessingManager::getPostProcessingTargets() const
{
    std::unique_lock<std::mutex> lock(registryMutex);

    std::vector<PostProcessingTargetInfo> result;
    result.reserve(targets.size());
    for (const auto& entry : targets) {
        PostProcessingTargetInfo info;
        info.name = entry.first;
        auto it = descriptions.find(entry.first);
        info.description = (it != descriptions.end()) ? it->second : std::string();
        result.push_back(info);
    }
    return result;
}

bool LocalPostProcessingManager::getTarget(const std::string& name, PostProcessingFunction& function) const
{
    std::unique_lock<std::mutex> lock(registryMutex);
    auto it = targets.find(name);
    if (it == targets.end()) {
        return false;
    }
    function = it->second;   //copy out so the callback runs without holding the registry lock
    return true;
}

bool LocalPostProcessingManager::requestPostProcessing(const std::string& name,
                                                       const ShotID& shotID,
                                                       const DeviceID& shotOwnerID,
                                                       const MetaData& options)
{
    {
        std::unique_lock<std::mutex> lock(registryMutex);
        if (targets.find(name) == targets.end()) {
            return false;   //not a registered target on this device; caller logs
        }
    }

    PostProcessWorkItem item;
    item.targetName = name;
    item.shotID = shotID;
    item.shotOwnerID = shotOwnerID;
    item.options = options;

    addEvent(item);   //non-blocking; worker is already running (a target is registered)
    return true;
}

void LocalPostProcessingManager::stop()
{
    //EventQueue::stop() stops accepting work, wakes the worker, and joins it.
    //Note: a worker mid-callback in user Python code holds the GIL; the pybind
    //wrapper must release the GIL around stop() to avoid a join deadlock.
    STI::Utils::EventQueue<PostProcessWorkItem>::stop();
}

bool LocalPostProcessingManager::resolveOwnerPersistenceManager(const DeviceID& shotOwnerID,
                                                               std::shared_ptr<PersistenceManager>& ownerPM) const
{
    //Resolve the owning device's PersistenceManager (local or remote via the
    //abstract Device interface). A remote owner is reachable iff it was declared
    //as a partner on this device (addPartner puts a direct reference in the
    //collection, independent of server-chain locality).
    if (shotOwnerID == deviceID || shotOwnerID.empty()) {
        ownerPM = persistenceManager;
        return ownerPM != 0;
    }

    if (deviceCollection != 0) {
        std::shared_ptr<Device> dev;
        if (deviceCollection->get(shotOwnerID, dev) && dev != 0) {
            dev->getPersistenceManager(ownerPM);
        }
    }
    return ownerPM != 0;
}

void LocalPostProcessingManager::handleEvent(const PostProcessWorkItem& item)
{
    PostProcessingFunction function;
    if (!getTarget(item.targetName, function)) {
        //Target was removed between request and execution; nothing to dispatch.
        if (logger != 0) {
            (*logger) << "PostProcessing: target '" << item.targetName << "' no longer registered\n";
        }
        return;
    }

    //Resolve the owner's PersistenceManager and pull the ShotResult on this worker
    //thread (off the play path), then hand the result to the user callback. The two
    //failure modes get distinct messages so the user can tell whether to fix their
    //addPartner configuration or look at why the shot data is missing.
    std::shared_ptr<PersistenceManager> ownerPM;
    if (!resolveOwnerPersistenceManager(item.shotOwnerID, ownerPM)) {
        dispatchComplete(item, PostProcessingStatus::Failed, MetaData(),
            "post-processing owner device '" + item.shotOwnerID.getID() +
            "' is not reachable; declare it as a partner (addPartner) on the post-processing device");
        return;
    }

    std::shared_ptr<ShotResult> shotResult;
    if (!ownerPM->getShotResult(item.shotID, shotResult) || shotResult == 0) {
        dispatchComplete(item, PostProcessingStatus::Failed, MetaData(),
            "shot result '" + item.shotID.print() + "' not found on owner device '" + item.shotOwnerID.getID() + "'");
        return;
    }

    MetaData results;
    PostProcessingStatus status = PostProcessingStatus::Success;
    std::string errorMessage;

    try {
        results = function(shotResult, item.options);
    }
    catch (const std::exception& e) {
        status = PostProcessingStatus::Failed;
        errorMessage = e.what();
    }
    catch (...) {
        status = PostProcessingStatus::Failed;
        errorMessage = "Unknown exception in post-processing callback";
    }

    dispatchComplete(item, status, results, errorMessage);
}

void LocalPostProcessingManager::dispatchComplete(const PostProcessWorkItem& item,
                                                  PostProcessingStatus status,
                                                  const MetaData& results,
                                                  const std::string& errorMessage)
{
    if (dispatcher == 0) {
        return;
    }

    STI::Device::DeviceTrace trace(deviceID);
    auto mess = std::make_shared<PostProcessingCompleteMessage>(trace);
    mess->shotID = item.shotID;
    mess->targetName = item.targetName;
    mess->status = status;
    if (status == PostProcessingStatus::Success) {
        mess->results = results;
    }
    else {
        mess->errorMessage = errorMessage;
    }

    dispatcher->addMessage(mess);
}
