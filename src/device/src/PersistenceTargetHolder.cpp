#include "PersistenceTargetHolder.h"
#include "PersistenceTarget.h"

using STI::Device::PersistenceTargetHolder;
using STI::Utils::ConfigFile;

PersistenceTargetHolder::PersistenceTargetHolder(const std::string& filename, const std::shared_ptr<PersistenceTarget>& target)
: target(target)
{
    saveMessager.setWarmup(500);    //ms
    saveMessager.setCooldown(500);    //ms
    saveMessager.start();

    file = std::make_shared<ConfigFile>(filename);

    if (target != 0) {
        // auto refresher = [this](void) -> void { return this->save(); };
        auto refresher = 
            [this](void) -> void 
            {
                //Add a save message to the messager (groups and delays incoming requests to limit save rate)
                auto saveMessage = std::make_shared<PersistenceTargetHolder::SaveMessage>(this);
                return this->saveMessager.addMessage(saveMessage);
            };

        target->setPersistenceCallback(refresher);
        target->setPersistenceData(file);
    }
}

PersistenceTargetHolder::~PersistenceTargetHolder()
{
    saveMessager.stop();
}

void PersistenceTargetHolder::load()
{
    if (target != 0) {
        target->load(); //load data from file in PersistenceTarget
    }
}

void PersistenceTargetHolder::save()
{
    if(file != 0 && target != 0 && target->save()) {
        file->setHeader(target->getHeader());
        file->save();
    }
}

