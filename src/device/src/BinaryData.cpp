#include <sti/utils/BinaryData.h>
#include <sti/utils/BinaryDataStream.h>

#include <algorithm>
#include <cmath>
#include <memory>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>

using STI::Utils::BinaryData;

namespace
{
class MaterializingBinaryDataStreamTarget : public STI::Utils::BinaryDataStreamTarget
{
public:
    explicit MaterializingBinaryDataStreamTarget(const std::shared_ptr<BinaryData>& target)
        : target(target)
    {
    }

    void start() override
    {
        started = true;
        stopped = false;
        chunks.clear();
    }

    void writeNext(const std::shared_ptr<BinaryData>& data) override
    {
        if (data != 0) {
            chunks.push_back(data);
        }
    }

    void stop() override
    {
        if (target != 0) {
            target->merge(chunks);
        }
        stopped = true;
    }

    bool started{false};
    bool stopped{false};

private:
    std::shared_ptr<BinaryData> target;
    std::vector<std::shared_ptr<BinaryData>> chunks;
};
} // namespace


BinaryData::BinaryData()
: isOwner(false), length_(0), wordSize_(0), data_(nullptr)
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
    }
    isOwner = false;
    length_ = 0;
    wordSize_ = 0;
    data_ = nullptr;
    streams.clear();
    clearer = nullptr;
    getType = nullptr;
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

bool BinaryData::hasLocalData() const
{
    return data_ != nullptr;
}

bool BinaryData::hasStream() const
{
    return std::any_of(streams.begin(), streams.end(),
                       [](const auto& stream) { return stream != nullptr; });
}

bool BinaryData::isMaterialized() const
{
    return hasLocalData();
}

void BinaryData::swap(BinaryData& other)
{
    std::swap(isOwner, other.isOwner);
    std::swap(length_, other.length_);
    std::swap(wordSize_, other.wordSize_);
    std::swap(data_, other.data_);
    std::swap(streams, other.streams);

    std::swap(clearer, other.clearer);
    std::swap(getType, other.getType);
}

bool BinaryData::getBytes(char*& data) const
{
    if (data_ == nullptr && hasStream()) {
        const_cast<BinaryData*>(this)->materialize();
    }
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
        if (chunk != 0) {
            totalSize += chunk->bytes();
        }
    }

    allocate<char>(totalSize);

    char* next;
    char* data;
    getBytes(data);

    size_t pos = 0;
    size_t chunkLength = 0;     //in bytes

    //deep copy
    for (auto& chunk : chunks) {
        if (chunk == 0 || !chunk->getBytes(next)) {
            continue;
        }

        chunkLength = chunk->bytes();
        if (chunkLength > 0) {
            std::copy(next, next + chunkLength, data + pos);
            pos += chunkLength;
        }
    }
}

void BinaryData::split(std::vector<std::shared_ptr<BinaryData>>& chunks, size_t maxBytes) const
{
    chunks.clear();

    if (maxBytes == 0) {
        return;
    }

    char* data;
    if (!getBytes(data)) {
        return;
    }

    size_t pos = 0;
    size_t remaining = bytes();

    while (remaining > 0) {
        auto chunk = std::make_shared<BinaryData>();

        size_t chunkLength = std::min(maxBytes, remaining);
        char* dataStart = static_cast<char*>(data + pos);
        chunk->assign(dataStart, chunkLength, false);

        pos += chunkLength;
        remaining -= chunkLength;
        chunks.push_back(chunk);
    }
}

void BinaryData::setMetadata(size_t length, size_t wordsize)
{
    length_ = length;
    wordSize_ = wordsize;
}

void BinaryData::attachStream(const std::shared_ptr<BinaryDataStream>& stream)
{
    if (stream != 0) {
        streams.push_back(stream);
    }
}

void BinaryData::attachStream(const std::shared_ptr<BinaryDataStream>& stream,
                              size_t length,
                              size_t wordsize)
{
    setMetadata(length, wordsize);
    attachStream(stream);
}

bool BinaryData::materialize()
{
    if (hasLocalData()) {
        return true;
    }
    if (!hasStream()) {
        return false;
    }

    auto localData = std::make_shared<BinaryData>();
    auto target = std::make_shared<MaterializingBinaryDataStreamTarget>(localData);

    auto stream = streams.back();
    if (stream == 0) {
        return false;
    }

    stream->transfer(target);

    if (!target->stopped) {
        return false;
    }

    if (!localData->hasLocalData()) {
        if (bytes() != 0) {
            return false;
        }
        localData->allocate<char>(0);
    }

    localData->setMetadata(length_, wordSize_);
    swap(*localData);
    return true;
}

bool BinaryData::transferTo(const std::shared_ptr<BinaryDataStreamTarget>& target)
{
    if (target == 0) {
        return false;
    }

    if (hasLocalData()) {
        std::vector<std::shared_ptr<BinaryData>> chunks;
        split(chunks, bytes() == 0 ? 1 : bytes());

        target->start();
        for (auto& chunk : chunks) {
            target->writeNext(chunk);
        }
        target->stop();
        return true;
    }

    if (hasStream()) {
        auto stream = streams.back();
        if (stream == 0) {
            return false;
        }
        stream->transfer(target);
        return true;
    }

    return false;
}


template<class Archive>
void BinaryData::save(Archive& archive) const
{
    char* data;
    getBytes(data);
    auto serialBinData = cereal::binary_data(data, bytes());

    //std::stringstream d;

	archive(
		cereal::make_nvp("isOwner", isOwner), 
        cereal::make_nvp("length_", length_), 
        cereal::make_nvp("wordSize_", wordSize_)
        //cereal::make_nvp("d", d.str())
        //cereal::make_nvp("data", serialBinData)
		);
}

template void BinaryData::save<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&) const;
template void BinaryData::save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&) const;


template<class Archive>
void BinaryData::load(Archive& archive)
{
    char* data;
    auto serialBinData = cereal::binary_data(data, 0);

    archive(
        cereal::make_nvp("isOwner", isOwner),
        cereal::make_nvp("length_", length_),
        cereal::make_nvp("wordSize_", wordSize_)
        //cereal::make_nvp("data", serialBinData)
    );
    data = static_cast<char*>(serialBinData.data);
    assign(data, serialBinData.size);
}


template void BinaryData::load<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
template void BinaryData::load<cereal::JSONInputArchive>( cereal::JSONInputArchive& );
