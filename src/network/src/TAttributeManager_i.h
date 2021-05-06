#ifndef STI_TNETWORK_TATTRIBUTEMANAGER_I_H
#define STI_TNETWORK_TATTRIBUTEMANAGER_I_H

#include "AttributeManager.h"

#include "Device.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TAttributeManager_i : public POA_STI::TNetwork::TAttributeManager
{
public:

	TAttributeManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TAttributeManager_i();

    char* getValue(const char* key);
    ::CORBA::Boolean setValue(const char* key, const char* value);
    void refreshValue(const char* key);
    // TMixedValue* getMetaDataAll(const char* key);
    // TMixedValue* getMetaData(const char* key, const char* metaKey);
    ::CORBA::Boolean getAttribute(const char* key, ::STI::TNetwork::TAttribute_out attrib);
    void getAttributes(::STI::TNetwork::TAttributeSeq_out attributes);
    ::CORBA::Boolean ping();
    
private:

    std::shared_ptr<STI::Device::AttributeManager> attributeManager;

};


} //TNetwork
} //STI

#endif

