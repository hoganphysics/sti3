#include <sti/utils/BinaryData.h>

#include <algorithm>
#include <cmath>

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

bool BinaryData::getBytes(char*& data) const
{
    data = static_cast<char*>(data_);
    return (data != 0);
}

bool BinaryData::getBytes(char*& data, bool orphan)
{
    //release if orphan=true
    
    if (getBytes(data)) {
        if (orphan) {
            isOwner = false;
        }
        return true;
    }
    return false;
}

void BinaryData::merge(std::vector<std::shared_ptr<BinaryData>>& chunks)
{
    size_t totalSize = 0;
    for (auto& chunk : chunks) {
        totalSize += chunk->bytes();
    }

    if (isType<char*>()) {
        allocate<char>(totalSize);
    }
    else if (isType<unsigned char*>()) {
        allocate<unsigned char>(totalSize);
    }
    else if (isType<signed char*>()) {
        allocate<signed char>(totalSize);
    }
    else if (isType<unsigned short*>()) {
        allocate<unsigned short>(totalSize);
    }
    else if (isType<short*>()) {
        allocate<short>(totalSize);
    }
    else if (isType<unsigned int*>()) {
        allocate<unsigned int>(totalSize);
    }
    else if (isType<int*>()) {
        allocate<int>(totalSize);
    }
    else if (isType<float*>()) {
        allocate<float>(totalSize);
    }
    else if (isType<double*>()) {
        allocate<double>(totalSize);
    }
    else {
        allocate<char>(totalSize);
    }

    char* next;
    char* data;
    getBytes(data);

    size_t pos = 0;
    size_t chunkLength = 0;     //in bytes

    //deep copy
    for (auto& chunk : chunks) {
        chunk->getBytes(next);
        chunkLength = chunk->bytes();

        std::copy(next, next + chunkLength, data + pos);
        pos += chunkLength;
    }
}

void BinaryData::split(std::vector<std::shared_ptr<BinaryData>>& chunks, size_t maxBytes) const
{
    chunks.clear();

    size_t nChunks = std::ceil((1.0 * bytes()) / maxBytes);     //number of chunks
    size_t maxChunk = std::ceil((1.0 * maxBytes) / wordsize()); //maximum of words per chunk

    size_t pos = 0;
    size_t chunkLength = 0;         //in words
    size_t remaining = length();    //in words

    char* data;
    getBytes(data);

    bool first = true;

    for (unsigned i = 0; i < nChunks && remaining > 0; ++i) {
        auto chunk = std::make_shared<BinaryData>();

        chunkLength = std::min(maxChunk, remaining);

        if (isType<char*>()) {
            char* dataStart = static_cast<char*>(data + pos);
            chunk->assign(dataStart, chunkLength, false);   //first owns the set: isOwner && transferOwnership && first
        }

        pos += (chunkLength * wordsize());
        remaining -= chunkLength;
        first = false;

        chunks.push_back(chunk);
    }
}

void BinaryData::attachStream(const std::shared_ptr<BinaryDataStream>& stream)
{
    streams.push_back(stream);
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

template void BinaryData::serialize<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& );
template void BinaryData::serialize<cereal::JSONInputArchive>( cereal::JSONInputArchive& );
