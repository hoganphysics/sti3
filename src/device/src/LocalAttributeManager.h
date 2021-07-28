#ifndef STI_DEVICE_LOCALATTRIBUTEMANAGER_H
#define STI_DEVICE_LOCALATTRIBUTEMANAGER_H

#include "AttributeManager.h"
#include "AttributeRefreshListener.h"

#include "DeviceID.h"
#include "SynchronizedMap.h"
#include "MessageGrouper.h"

#include <map>
#include <vector>
#include <string>
#include <memory>


namespace STI
{
namespace Device
{

class Attribute;
class LocalAttribute;
class DeviceID;
class DeviceMessageDispatcher;
class AttributeUpdateMessage;


class LocalAttributeManager : public AttributeManager,
                              public AttributeRefreshListener
{
public:

    LocalAttributeManager(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
    ~LocalAttributeManager();
   
    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes);

    void getAttributes(std::map<std::string, std::string>& attributes);

    bool addAttribute(const std::shared_ptr<LocalAttribute>& attribute);

private:

    void handleAttributeRefreshEvent(const std::string& key, const std::string& value);

    const DeviceID localID;		//this device's DeviceID

    STI::Utils::SynchronizedMap<std::string, std::shared_ptr<LocalAttribute>> attributeMap;

    STI::Device::MessageGrouper<AttributeUpdateMessage> messageGrouper;

};


} //Device
} //STI

#endif

