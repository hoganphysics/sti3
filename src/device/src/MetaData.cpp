
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

using STI::Utils::MetaData;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

MetaData::MetaData()
{
	std::vector<MixedValue> vec;
	metaData.setValue(vec);
}

MetaData::MetaData(const STI::Utils::MixedValue& data)
{
	std::vector<MixedValue> vec;
	metaData.setValue(vec);
	
	if (!data.isType(MixedValueType::Vector)) {
		return;
	}

	const STI::Utils::MixedValueVector& values = data.getVector();

	for (auto& tuple : values) {
		if (isTuple(tuple)) {
			
			addMetaData( 
				tuple.getVector().at(0).getString(), 	//key
				tuple.getVector().at(1)				 	//value
				);
		}
	}
}

MetaData::~MetaData()
{
}

bool MetaData::contains(const std::string& key) const
{
    bool found = false;

	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		if (tupleMatch(tuple, key)) {
			found = true;
			break;
		}
	}

    return found;
}

void MetaData::addMetaData(const std::string& key, const STI::Utils::MixedValue& value)
{
    if (contains(key) && resetMetaDataEntry(key, value)) {   //if key already exists, overwrite
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

	STI::Utils::MixedValue newMetaData;
	const STI::Utils::MixedValueVector& values = metaData.getVector();

	for (auto& tuple : values) {
		STI::Utils::MixedValue labeledData;
		labeledData.clear();
	
		if (tupleMatch(tuple, key)) {
			labeledData.addValue( key ); 		//key
			labeledData.addValue( newValue );	//value

			newMetaData.addValue(labeledData);
			success = true;
		}
		else if (isTuple(tuple)) {
			labeledData.addValue( tuple.getVector().at(0) ); //key
			labeledData.addValue( tuple.getVector().at(1) ); //value

			newMetaData.addValue(labeledData);
		}
	}

	if (success) {
		metaData = newMetaData;
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
		if (tupleMatch(tuple, key)) {
			data = tuple.getVector().at(1);
			break;
		}
	}

	return data;
}

std::vector<std::string> MetaData::keys() const
{
	std::vector<std::string> keylist;

	for (auto& labeledData : metaData.getVector()) {
		if (isTuple(labeledData)) {
			keylist.push_back( labeledData.getVector().at(0).getString() );
		}
	}
	return keylist;
}

void MetaData::merge(const MetaData& data)
{
	for (auto& labeledData : data.getMetaData().getVector()) {
		if (isTuple(labeledData)) {
			addMetaData(
				labeledData.getVector().at(0).getString(),
				labeledData.getVector().at(1)
			);
		}
	}
}

bool MetaData::isTuple(const STI::Utils::MixedValue& tuple)
{
	if (tuple.isType(MixedValueType::Vector)) {
		const STI::Utils::MixedValueVector& labeledData = tuple.getVector();

		return (labeledData.size() == 2 && labeledData.at(0).isType(MixedValueType::String));
	}
	return false;
}


bool MetaData::tupleMatch(const STI::Utils::MixedValue& tuple, const std::string& key)
{
	return (isTuple(tuple) && tuple.getVector().at(0).getString().compare(key) == 0);
}

