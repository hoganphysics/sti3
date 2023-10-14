#ifndef STI_UTILS_BINARYDATA_H
#define STI_UTILS_BINARYDATA_H

#include <functional>
#include <typeinfo>
#include <vector>
#include <memory>


namespace STI
{
namespace Utils
{

class BinaryData;
class BinaryDataStream;


class BinaryData
{
public:

    template<typename T>
    BinaryData(T*& data, size_t length);

    BinaryData();
    ~BinaryData();
    
    BinaryData(const BinaryData&) = delete;
    void operator=(const BinaryData&) = delete;

    bool operator==(const BinaryData& other) const;
	bool operator!=(const BinaryData& other) const;

    void clear();

    size_t length() const;
    size_t bytes() const;
    size_t wordsize() const;

    template<typename T>
    bool isType() const;

    template<typename T>
    bool get(T*& data, bool orphan);    //release if orphan=true
    
    template<typename T>
    bool get(T*& data) const;

    bool getBytes(char*& data) const;
    bool getBytes(char*& data, bool orphan);    //release if orphan=true

    template<typename T>
    void assign(T*& data, size_t length, bool takeOwnership=true);

    template<typename T>
    T* allocate(size_t length);

    void split(std::vector<std::shared_ptr<BinaryData>>& chunks, size_t maxBytes) const;
    void merge(std::vector<std::shared_ptr<BinaryData>>& chunks);

    void swap(BinaryData& other);

	template<class Archive>
	void save(Archive& archive) const;

    template<class Archive>
    void load(Archive& archive);
    
    void attachStream(const std::shared_ptr<BinaryDataStream>& stream);


    //void test()
    //{
    //    //in NetworkConvert
    //    if (bytes() > 1000) {
    //        std::vector<std::shared_ptr<BinaryData>> chunks;
    //        split(chunks, 1000, true);
    //    }
    //}

    //class BinaryDataTarget  //on remote, receives data
    //{
    //    BinaryDataTarget(const std::shared_ptr<BinaryData>& target);

    //    bool allocate(size_t wordSize, size_t length);

    //    template<typename T>
    //    void write(T*& data, size_t start, size_t length);
    //};

    //class BinaryDataServer  //on local, pointing to source data to send
    //{
    //    BinaryDataServer(BinaryData* source);
    //    void transfer(const std::shared_ptr<BinaryDataTarget>& destination);
    //};

    //void attachServer(const std::shared_ptr<BinaryDataServer>& server)
    //{

    //}

    //friend BinaryDataServer;
    //std::vector<std::shared_ptr<BinaryDataServer>> servers;

private:  

    bool isOwner;
    size_t length_;
    size_t wordSize_;
    void* data_;

    std::vector<std::shared_ptr<BinaryDataStream>> streams;

    std::function<void(void)> clearer;
    std::function<const std::type_info&(void)> getType;
};


} //Utils
} //STI



template<typename T>
STI::Utils::BinaryData::BinaryData(T*& data, size_t length)
{
    assign(data, length);
}

template<typename T>
bool STI::Utils::BinaryData::isType() const
{
    return getType && typeid(T) == getType();
}

template<typename T>
bool STI::Utils::BinaryData::get(T*& data, bool orphan)
{
    if (get(data)) {
        if (orphan) {
            isOwner = false;
        }
        return true;
    }
    return false;
}

template<typename T>
bool STI::Utils::BinaryData::get(T*& data) const
{
    if (getType && getType() == typeid(data)) {
        data = static_cast<T*>(data_);
        return (data != 0);
    }
    return false;
}

template<typename T>
void STI::Utils::BinaryData::assign(T*& data, size_t length, bool takeOwnership)
{
    isOwner = takeOwnership;
    data_ = static_cast<void*>(data);
    wordSize_ = sizeof(T);
    length_ = length;

    if (length > 0) {
        clearer = [data]()
        {
            T* tData = data;
            delete [] tData;
        };            
    }
    else {
        clearer = [data]()
        {
            T* tData = data;
            delete tData;
        }; 
    }

    getType = [data]() -> const std::type_info&
    {
        return typeid(data);
    };
}

template<typename T>
T* STI::Utils::BinaryData::allocate(size_t length)
{
    clear();
    T* tData = new T[length];
    assign(tData, length);
    return tData;
}


#endif
