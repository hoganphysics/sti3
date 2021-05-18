
#include "Convert_Attribute.h"
#include "RemoteAttribute.h"
#include "MixedValue.h"

#include "orbTypes.h"

#include "NetworkConvert.h"

using STI::Network::convert;
using STI::Device::Attribute; 
using STI::TNetwork::TAttribute;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::Network::RemoteAttribute; 


//Attribute
template<>
bool STI::Network::convert<std::shared_ptr<Attribute>, TAttribute>(const std::shared_ptr<Attribute>& attribute, TAttribute& tAttribute)
{
    bool success = false;

    if (attribute != 0) {
        success = true;
        tAttribute.key = STI::Network::convert<std::string, ::CORBA::String_member>(attribute->getKey());
        tAttribute.value = STI::Network::convert<std::string, ::CORBA::String_member>(attribute->getValue());
        tAttribute.group = STI::Network::convert<std::string, ::CORBA::String_member>(attribute->getGroup());
        
        // STI::Network::convert<std::string, ::CORBA::String_member>(attribute->getAllowedValues(), tAttribute.allowedValues);

        STI::Network::convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(attribute->getAllowedValues(), tAttribute.allowedValues);
    }

    return success;
}


template<>
bool STI::Network::convert<TAttribute, std::shared_ptr<Attribute>>(const TAttribute& tAttribute, std::shared_ptr<Attribute>& attribute)
{
    attribute = STI::Network::convert<TAttribute, std::shared_ptr<Attribute>>(tAttribute);

    return (attribute != 0);
}

template<>
std::shared_ptr<Attribute> STI::Network::convert<TAttribute, std::shared_ptr<Attribute>>(const TAttribute& tAttribute)
{
    auto attribute = STI::Network::convert<TAttribute, std::shared_ptr<RemoteAttribute>>(tAttribute);
    return std::static_pointer_cast<Attribute>(attribute);
}


//RemoteAttribute
template<>
bool STI::Network::convert<TAttribute, std::shared_ptr<RemoteAttribute>>(const TAttribute& tAttribute, std::shared_ptr<RemoteAttribute>& attribute)
{
    attribute = STI::Network::convert<TAttribute, std::shared_ptr<RemoteAttribute>>(tAttribute);

    return (attribute != 0);
}

template<>
std::shared_ptr<RemoteAttribute> STI::Network::convert<TAttribute, std::shared_ptr<RemoteAttribute>>(const TAttribute& tAttribute)
{
    std::vector<std::string> allowedValues;
    STI::Network::convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tAttribute.allowedValues, allowedValues);
    // convert<::CORBA::String_member, std::string>(tAttribute.allowedValues, allowedValues);

    auto remoteAttribute = std::make_shared<STI::Network::RemoteAttribute>(
                                convert<::CORBA::String_member, std::string>(tAttribute.key),
                                convert<::CORBA::String_member, std::string>(tAttribute.value),
                                convert<::CORBA::String_member, std::string>(tAttribute.group),
                                allowedValues, convert<TMixedValue, MixedValue>(tAttribute.metaData)
    );

    return remoteAttribute;
}
