
#include "MetaData.h"
#include "MixedValue.h"

using STI::Utils::MetaData;
using STI::Utils::MixedValue;


MetaData::MetaData()
{
}

MetaData::~MetaData()
{
}

bool MetaData::contains(const std::string& key) const
{
    bool found = false;

	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		const STI::Utils::MixedValueVector& labeledData = tuple.getVector();

		if (labeledData.size() > 0 && labeledData.at(0).getString().compare(key) == 0) {
			found = true;
			break;
		}
	}
    return found;
}

void MetaData::addMetaData(const std::string& key, const STI::Utils::MixedValue& value)
{
    if (resetMetaDataEntry(key, value)) {   //if key already exists, overwrite
        return;
    }
    //else add new key/value pair

	STI::Utils::MixedValue labeledData;
	labeledData.addValue(key);
	labeledData.addValue(value);

	metaData.addValue(labeledData);
}

bool MetaData::resetMetaDataEntry(const std::string& key, const STI::Utils::MixedValue& newValue)
{
    bool success = false;

	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		const STI::Utils::MixedValueVector& labeledData = tuple.getVector();

		if (labeledData.size() == 2 && labeledData.at(0).getString().compare(key) == 0) {
			//labeledData.at(1).setValue(newValue);
            //success = true;
			break;
		}
	}

    return success;
}

const STI::Utils::MixedValue& MetaData::getMetaData() const
{
	return metaData;
}

STI::Utils::MixedValue MetaData::getMetaData(const std::string& key) const
{
	STI::Utils::MixedValue data;	//empty

	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		const STI::Utils::MixedValueVector& labeledData = tuple.getVector();
		if (labeledData.size() == 2 && labeledData.at(0).getString().compare(key) == 0) {
			data = labeledData.at(1);
			break;
		}
	}

	return data;
}

