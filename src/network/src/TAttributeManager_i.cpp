
#include "TAttributeManager_i.h"
#include "ORBManager.h"
#include "AttributeManager.h"
#include "Convert_Attribute.h"
#include "Attribute.h"

using STI::TNetwork::TAttributeManager_i;

using STI::Network::convert;
using STI::TNetwork::TAttributeSeq;
using STI::Device::Attribute;
using STI::TNetwork::TAttribute;


TAttributeManager_i::TAttributeManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getAttributeManager(attributeManager);        
    }
}

TAttributeManager_i::~TAttributeManager_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

char* TAttributeManager_i::getValue(const char* key)
{
    std::string value = "";

    if (attributeManager != 0) {

		value = attributeManager->getValue(convert<CORBA::String_member, std::string>(key));
	}

    return CORBA::string_dup( convert<std::string, CORBA::String_member>(value) );
}

::CORBA::Boolean TAttributeManager_i::setValue(const char* key, const char* value)
{
    bool success = false;

    if (attributeManager != 0) {

		success = attributeManager->setValue(
            convert<CORBA::String_member, std::string>(key),
            convert<CORBA::String_member, std::string>(value));
	}

    return success;
}

void TAttributeManager_i::refreshValue(const char* key)
{
    if (attributeManager != 0) {

        std::shared_ptr<Attribute> attribute;

        if (attributeManager->getAttribute(convert<CORBA::String_member, std::string>(key), attribute)) {
            attribute->refreshValue();
        }
	}
}

::CORBA::Boolean TAttributeManager_i::getAttribute(const char* key, ::STI::TNetwork::TAttribute_out attrib)
{
	bool success = false;

    if (attributeManager != 0) {

		STI::TNetwork::TAttribute_var tAttribute_var(new STI::TNetwork::TAttribute);
		std::shared_ptr<Attribute> localAttribute;

		success = attributeManager->getAttribute(convert<CORBA::String_member, std::string>(key), localAttribute);

		convert<std::shared_ptr<Attribute>, TAttribute>(localAttribute, tAttribute_var);

		attrib = new STI::TNetwork::TAttribute();
		(*attrib) = tAttribute_var;
	}
	return success;
}

void TAttributeManager_i::getAttributes(::STI::TNetwork::TAttributeSeq_out attributes)
{
    if (attributeManager != 0) {

		STI::TNetwork::TAttributeSeq_var tAttributeSeq_var(new STI::TNetwork::TAttributeSeq);
		std::vector<std::shared_ptr<Attribute>> localAttributes;

		attributeManager->getAttributes(localAttributes);

		convert<std::shared_ptr<Attribute>, TAttribute>(localAttributes, tAttributeSeq_var);

		attributes = new STI::TNetwork::TAttributeSeq();
		(*attributes) = tAttributeSeq_var;
	}
}

::CORBA::Boolean TAttributeManager_i::ping()
{
    return true;
}

