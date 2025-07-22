#include "LocalAttributeManager.h"

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/LocalAttribute.h>

#include <sti/utils/ConfigFile.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/TimeStamp.h>

#include <set>
#include <sstream>


using STI::Device::LocalAttributeManager;
using STI::Device::Attribute;
using STI::Device::LocalAttribute;
using STI::Device::Attribute;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageDispatcher;
using STI::Utils::ConfigFile;
using STI::Utils::Configuration;
using STI::Device::Profile;


LocalAttributeManager::LocalAttributeManager(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
 : localID(localID), messageGrouper(dispatcher), loading(false)
{
    messageGrouper.setWarmup(100);   //ms
    messageGrouper.setCooldown(500); //ms

    messageGrouper.start();

    file = std::make_shared<ConfigFile>();
}

LocalAttributeManager::~LocalAttributeManager()
{
}


std::string LocalAttributeManager::getValue(const std::string& key)
{
    std::shared_ptr<Attribute> attribute;

    if (getAttribute(key, attribute) && attribute != 0) {
        return attribute->getValue();
    }
    return "";
}


bool LocalAttributeManager::getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute)
{
    std::shared_ptr<LocalAttribute> localAttribute;

    if (attributeMap.get(key, localAttribute)) {
        attribute = localAttribute;
        return (attribute != 0);
    }
    return false;
}


void LocalAttributeManager::getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes)
{
    attributes.clear();

    std::set<std::string> keys;
    std::shared_ptr<Attribute> attribute;

    attributeMap.getKeys(keys);

    for (auto& key : keys) {
        if (getAttribute(key, attribute) && attribute != 0) {
            attributes.push_back(attribute);
        }
    }
}

void LocalAttributeManager::getAttributes(std::map<std::string, std::string>& attributes)
{
    attributes.clear();

    std::set<std::string> keys;
    attributeMap.getKeys(keys);

    std::shared_ptr<Attribute> attribute;
    
    for (auto& key : keys) {
        if (getAttribute(key, attribute) && attribute != 0) {
            attributes[attribute->getKey()] = attribute->getValue();
        }
    }
}

bool LocalAttributeManager::setValue(const std::string& key, const std::string& value)
{
    std::shared_ptr<Attribute> attribute;

    if (getAttribute(key, attribute) && attribute != 0) {
        return attribute->setValue(value);
    }
    return false;
}


bool LocalAttributeManager::addAttribute(const std::shared_ptr<LocalAttribute>& attribute)
{
    if (attribute != 0 && attributeMap.add(attribute->getKey(), attribute)) {
        attribute->addRefreshListener(this);

        //add any new keys to persistence
        if (persistenceData != 0 && !persistenceData->includes("Attributes", attribute->getKey())) {           
            persistenceData->set("Attributes", attribute->getKey(), attribute->getValue());
        }
        return true;
    }
    return false;
}


void LocalAttributeManager::handleAttributeRefreshEvent(const std::string& key, const std::string& value)
{
    auto message = std::make_shared<STI::Device::AttributeUpdateMessage>(localID, key, value);
    messageGrouper.addMessage(message);

    //persistence
    if (persistenceData != 0 && !loading.load()) {
        persistenceData->set("Attributes", key, value);

        if (persistenceRefresher) {
            persistenceRefresher();
        }
    }
}


bool LocalAttributeManager::loadProfile(const std::shared_ptr<Profile>& profile)
{
    if (profile == 0) return false;

    if (profile->type != ProfileType::All && profile->type != ProfileType::Attribute) return true;

    bool success = true;

    for (auto& attribute : profile->attributeData) {

        success &= setValue(attribute.first, attribute.second);
    }
    return success;

}

bool LocalAttributeManager::saveProfile(const std::shared_ptr<Profile>& profile)
{
    if (profile == 0) return false;

    if (profile->type != ProfileType::All && profile->type != ProfileType::Attribute) return true;

    getAttributes(profile->attributeData);

    return true;
}


//*********** PersistenceTarget ****************//

std::string LocalAttributeManager::getFilename()
{
    return "attributes.ini";
}

std::string LocalAttributeManager::getHeader()
{
    std::stringstream header;
    header << "Attributes for " << localID.getID() << std::endl;
    header << "  Name: " << localID.getName() << std::endl;
    header << "  Address: " << localID.getAddress() << std::endl;
    header << "  Module: " << localID.getModule() << std::endl;

    STI::Utils::TimeStamp timestamp;
    header << "Last saved: " << timestamp.print() << std::endl;

    return header.str();
}

void LocalAttributeManager::setPersistenceCallback(const std::function<void(void)>& refresher)
{
    persistenceRefresher = refresher;
}

bool LocalAttributeManager::save(const std::string& filename)
{
    if (file == 0 || !file->isParsed()) return false;

    file->setHeader(getHeader());
    file->save();   //persistenceData is kept current with each refresh event

    return true;
}

void LocalAttributeManager::load(const std::string& filename)
{
    if (file != 0 && !file->isParsed()) {
        
        file->load(filename, true);
    }
    else {
        file = std::make_shared<ConfigFile>(filename);
    }
    persistenceData = file;

    loading = true;

    if (persistenceData == 0) return;

    auto storedKeys = persistenceData->getParameterNames("Attributes");

    for (auto& key : storedKeys) {
        std::string value;
        if (persistenceData->getParameter("Attributes", key, value)) {
            setValue(key, value);
        }
    }
    loading = false;
}
