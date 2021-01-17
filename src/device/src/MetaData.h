#ifndef STI_UTILS_METADATA_H
#define STI_UTILS_METADATA_H

#include "MixedValue.h"


#include <string>


namespace STI
{
namespace Utils
{


class MetaData
{
public:

    MetaData();
	~MetaData();

    bool contains(const std::string& key) const;

    void addMetaData(const std::string& key, const STI::Utils::MixedValue& value);
    bool resetMetaDataEntry(const std::string& key, const STI::Utils::MixedValue& newValue);
    
    const STI::Utils::MixedValue& getMetaData() const;
    STI::Utils::MixedValue getMetaData(const std::string& key) const;

private:

    STI::Utils::MixedValue metaData;

};


} //Device
} //STI

#endif

