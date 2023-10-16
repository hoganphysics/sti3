#ifndef STI_DEVICE_PERSISTENCETARGET_H
#define STI_DEVICE_PERSISTENCETARGET_H

#include <sti/utils/Configuration.h>

#include <string>
#include <memory>
#include <functional>

namespace STI
{
namespace Device
{


class PersistenceTarget
{
public:

    virtual std::string getFilename() = 0;
    virtual void setPersistenceCallback(const std::function<void(void)>& refresher) = 0;
    virtual bool save(const std::string& filename) = 0;
    virtual void load(const std::string& filename) = 0;
};


} //Device
} //STI

#endif
