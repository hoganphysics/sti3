
#include "RemoteAttributeManager.h"
#include "Attribute.h"
#include "RemoteAttribute.h"
#include "DeviceMessageListenerForwarder.h"
#include "DeviceMessage.h"
#include "NetworkConvert.h"
#include "Convert_Attribute.h"

#include "deviceNet.h"


using STI::Network::RemoteAttributeManager;
using STI::Network::convert;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TAttribute;
using STI::Device::Attribute;
using STI::Network::RemoteAttribute;


RemoteAttributeManager::RemoteAttributeManager(::STI::TNetwork::TAttributeManager_ptr attributeManager,
                            const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                            const STI::Device::DeviceID& remoteID)
: tAttributeManager(STI::TNetwork::TAttributeManager::_duplicate(attributeManager)), listenerForwarder(forwarder), remoteID(remoteID)
{
	//Message listener for attribute update messages
	std::shared_ptr<STI::Device::DeviceMessageListener<STI::Device::AttributeUpdateMessage>> listener;
	listener = std::make_shared<AttributeUpdater>(this);
	
	listenerID.name = "RemoteAttributeManager::AttributeUpdate";
	listenerID.type = STI::Device::DeviceMessageType::AttributeUpdate;

	if (listenerForwarder != 0) {
		listenerForwarder->addListener(remoteID, listenerID, listener);		//Listen to events gemerated by remoteID
	}

	//Initialize attribute data
	std::vector<std::shared_ptr<Attribute>> attributes;
	getAttributes(attributes);
}

RemoteAttributeManager::~RemoteAttributeManager()
{
	if (listenerForwarder != 0) {
		listenerForwarder->removeListener(remoteID, listenerID);		
	}
}

void RemoteAttributeManager::disable()
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	::STI::TNetwork::TAttributeManager_var nilManager = ::STI::TNetwork::TAttributeManager::_nil();
	tAttributeManager = nilManager;	//release reference; reference is now nil
}

void RemoteAttributeManager::setAttributeData(const std::shared_ptr<STI::Device::Attribute>& attribute)
{
	if (attribute != 0) {
		attributeData[attribute->getKey()] = attribute->getValue();	
	}
}

std::string RemoteAttributeManager::getValue(const std::string& key)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

    std::string value = "";

	if (CORBA::is_nil(tAttributeManager)) return value;
    
	try {
		auto tValue = tAttributeManager->getValue(
                    convert<std::string, CORBA::String_member>(key));	//remote call

		value = convert<CORBA::String_member, std::string>(tValue);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    return value;
}

std::string RemoteAttributeManager::getUpdatedValue(const std::string& key)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);
	std::string value = "";

	auto it = attributeData.find(key);

	if (it != attributeData.end()) {
		return it->second;
	}
	return value;
}

bool RemoteAttributeManager::setValue(const std::string& key, const std::string& value)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (CORBA::is_nil(tAttributeManager)) return false;

    bool success = false;

    try {

		success = tAttributeManager->setValue(
                    convert<std::string, CORBA::String_member>(key), 
                    convert<std::string, CORBA::String_member>(value));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    return success;
}

bool RemoteAttributeManager::getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (CORBA::is_nil(tAttributeManager)) return false;

    bool success = false;
	STI::TNetwork::TAttribute_var tAttribute(new STI::TNetwork::TAttribute);
    std::shared_ptr<RemoteAttribute> remoteAttribute;

    try {

		tAttributeManager->getAttribute(
                    convert<std::string, CORBA::String_member>(key), tAttribute);	//remote call

		success = convert<TAttribute, std::shared_ptr<RemoteAttribute>>(tAttribute, remoteAttribute);

        if (success && remoteAttribute != 0) {
            // _attributes[key] = remoteAttribute;
            remoteAttribute->attachManager(this);
			setAttributeData(remoteAttribute);
            attribute = remoteAttribute;
        }
        
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

    return false;
}

void RemoteAttributeManager::getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (CORBA::is_nil(tAttributeManager)) return;

	std::vector<std::shared_ptr<RemoteAttribute>> remoteAttributes;

	STI::TNetwork::TAttributeSeq_var tAttributes(new STI::TNetwork::TAttributeSeq);

    try {

		tAttributeManager->getAttributes(tAttributes);	//remote call

		// convert<TAttribute, std::shared_ptr<Attribute>>(tAttributes, attributes);
		if (convert<TAttribute, std::shared_ptr<RemoteAttribute>>(tAttributes, remoteAttributes)) {
			
			attributes.clear();
			attributeData.clear();

			for (auto& ra : remoteAttributes) {
				if (ra != 0) {
					ra->attachManager(this);
					setAttributeData(ra);
					attributes.push_back(ra);					
				}
			}
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteAttributeManager::ping() const
{
	if (CORBA::is_nil(tAttributeManager)) return false;
	
	bool success = false;

	try {
		success = tAttributeManager->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

void RemoteAttributeManager::handleMessage(const std::shared_ptr<STI::Device::AttributeUpdateMessage>& mess)
{
	std::unique_lock<std::mutex> managerLock(managerMutex);

	if (mess == 0) return;

	for (auto& tuple : mess->attributes) {
		attributeData[tuple.first] = tuple.second;
	}
}

