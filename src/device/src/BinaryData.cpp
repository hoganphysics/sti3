#include <sti/utils/BinaryData.h>

#include <algorithm>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>

using STI::Utils::BinaryData;


BinaryData::BinaryData()
: isOwner(false), length_(0), wordSize_(0)
{
}

BinaryData::~BinaryData()
{
    clear();
}

void BinaryData::clear()
{
    if (isOwner && clearer) {
        clearer();
        isOwner = false;
    }
}

bool BinaryData::operator==(const BinaryData& other) const
{
    return other.data_ == data_;
}

bool BinaryData::operator!=(const BinaryData& other) const
{
    return !((*this) == other);
}

size_t BinaryData::length() const
{
    return length_;
}

size_t BinaryData::bytes() const
{
    return length_ * wordSize_;
}

size_t BinaryData::wordsize() const
{
    return wordSize_;
}

void BinaryData::swap(BinaryData& other)
{
    std::swap(isOwner, other.isOwner);
    std::swap(length_, other.length_);
    std::swap(wordSize_, other.wordSize_);
    std::swap(data_, other.data_);

    std::swap(clearer, other.clearer);
    std::swap(getType, other.getType);
}


template<class Archive>
void BinaryData::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("isOwner", isOwner), 
        cereal::make_nvp("length_", length_), 
        cereal::make_nvp("wordSize_", wordSize_)
		);
}


template void BinaryData::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void BinaryData::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
