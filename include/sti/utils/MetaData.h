#ifndef STI_UTILS_METADATA_H
#define STI_UTILS_METADATA_H

#include <sti/utils/MixedValue.h>

#include <string>
#include <vector>


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

    template<typename T>
    void addMetaData(const std::string& key, const T& value)
    {
        MixedValue mixedValue;
        mixedValue.setValue(value);
        addMetaData(key, (const STI::Utils::MixedValue&) value);
    }

    void addMetaData(const std::string& key, const STI::Utils::MixedValue& value);

    bool resetMetaDataEntry(const std::string& key, const STI::Utils::MixedValue& newValue);
    
    void removeMetaData(const std::string& key);
    void clear();

    const STI::Utils::MixedValue& getMetaData() const;
    STI::Utils::MixedValue getMetaData(const std::string& key) const;

    std::vector<std::string> keys() const;

    void merge(const MetaData& data);

private:

    static bool isTuple(const STI::Utils::MixedValue& tuple);
    static bool tupleMatch(const STI::Utils::MixedValue& tuple, const std::string& key);
    bool resetMetaDataEntry(const std::string& key, const STI::Utils::MixedValue& newValue, bool removeKey);

private:

    STI::Utils::MixedValue metaData;

};


} //Device
} //STI

#endif

