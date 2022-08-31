#ifndef STI_UTILS_METADATA_H
#define STI_UTILS_METADATA_H

#include <sti/utils/MixedValue.h>


#include <string>


namespace STI
{
namespace Utils
{


class MetaData
{
public:

    MetaData();
    MetaData(const STI::Utils::MixedValue& data);
	~MetaData();

    bool contains(const std::string& key) const;

    void addMetaData(const std::string& key, const STI::Utils::MixedValue& value);
    bool resetMetaDataEntry(const std::string& key, const STI::Utils::MixedValue& newValue);
    
    const STI::Utils::MixedValue& getMetaData() const;
    STI::Utils::MixedValue getMetaData(const std::string& key) const;

    std::vector<std::string> keys() const;

    void merge(const MetaData& data);

private:

    static bool isTuple(const STI::Utils::MixedValue& tuple);
    static bool tupleMatch(const STI::Utils::MixedValue& tuple, const std::string& key);

    STI::Utils::MixedValue metaData;

};


} //Device
} //STI

#endif

