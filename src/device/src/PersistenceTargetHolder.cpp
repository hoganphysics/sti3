#include "PersistenceTargetHolder.h"
#include "PersistenceTarget.h"
#include <filesystem>

namespace fs = std::filesystem;

using STI::Device::PersistenceTargetHolder;
using STI::Utils::ConfigFile;


PersistenceTargetHolder::PersistenceTargetHolder(const std::shared_ptr<PersistenceTarget>& target, const std::string& basepath)
: target(target), basepath(basepath)
{
    saveMessager.setWarmup(500);    //ms
    saveMessager.setCooldown(500);    //ms
    saveMessager.start();
}

void PersistenceTargetHolder::attachPersistenceCallback()
{
    if (target != 0) {
        std::weak_ptr<PersistenceTargetHolder> weakSelf = weak_from_this();
        auto refresher = [weakSelf](void) -> void {
            if (auto self = weakSelf.lock()) {
                self->requestSave();
            }
        };

        target->setPersistenceCallback(refresher);
    }
}

void PersistenceTargetHolder::requestSave()
{
    //Group and delay incoming requests to limit the persistence write rate.
    auto saveMessage = std::make_shared<PersistenceTargetHolder::SaveMessage>(this);
    saveMessager.addMessage(saveMessage);
}

PersistenceTargetHolder::~PersistenceTargetHolder()
{
    if (target != 0) {
        target->setPersistenceCallback([](){});
    }
    saveMessager.stop();
}

void PersistenceTargetHolder::load()
{
    if (target != 0) {
        std::filesystem::path filename(basepath);
        filename /= (target->getFilename());
        target->load(filename.string()); //load data from file in PersistenceTarget
    }
}

void PersistenceTargetHolder::save()
{
    if (target != 0) {

        std::filesystem::path filename(basepath);
        
        filename /= (target->getFilename());
        
        target->save(filename.string());
    }
}
