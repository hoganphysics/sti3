#ifndef STI_DEVICE_PERSISTENCETARGETHOLDER_H
#define STI_DEVICE_PERSISTENCETARGETHOLDER_H

#include <sti/utils/ConfigFile.h>
#include <sti/device/MessageGrouper.h>

#include <string>
#include <memory>
#include <functional>


namespace STI
{
namespace Device
{

class PersistenceTarget;

class PersistenceTargetHolder
{
public:

    PersistenceTargetHolder(const std::shared_ptr<PersistenceTarget>& target);
    virtual ~PersistenceTargetHolder();

    void load(const std::string& basePath);
    void save();

private:

    std::shared_ptr<PersistenceTarget> target;
    // std::shared_ptr<STI::Utils::ConfigFile> file;

    //Group refresh events to limit rate of writing to file
    class SaveMessage : public STI::Device::GroupableMessage<SaveMessage>
    {
    public:
        SaveMessage(PersistenceTargetHolder* self) : self(self) {}
        PersistenceTargetHolder* self;
        bool appendMessage(const SaveMessage& mess) { return true; }
        bool groupable() const { return true; }
        SaveMessage& get() { return *this; }
    };

    class SaveMessager : public STI::Device::MessageGrouper<SaveMessage>
    {
        //Executed after the MessageGrouper waits to warmup/cooldown
        void dispatchMessage(const std::shared_ptr<SaveMessage>& mess)
        {
            if (mess != 0 && mess->self != 0) {
                mess->self->save();
            }
        }
    };
    SaveMessager saveMessager;
};


} //Device
} //STI

#endif
