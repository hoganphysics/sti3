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

    // file = std::make_shared<ConfigFile>(filename);

    if (target != 0) {
        // auto refresher = [this](void) -> void { return this->save(); };
        auto refresher = 
            [this](void) -> void 
            {
                //Add a save message to the messager (groups and delays incoming requests to limit save rate)
                auto saveMessage = std::make_shared<PersistenceTargetHolder::SaveMessage>(this);
                return saveMessager.addMessage(saveMessage);
            };

        target->setPersistenceCallback(refresher);
        // target->setPersistenceData(file);
        // target->setLoadFilename(filename);
    }
}

PersistenceTargetHolder::~PersistenceTargetHolder()
{
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
    // if(file != 0 && target != 0 && target->save()) {
    //     file->setHeader(target->getHeader());
    //     file->save();
    // }

    if (target != 0) {
        std::filesystem::path filename(basepath);
        filename /= (target->getFilename());
        
        target->save(filename.string());
    }
}

