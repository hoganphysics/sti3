#include <sti/utils/PayloadUtils.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/FileHolder.h>

#include <cstring>
#include <limits>

namespace STI
{
namespace Utils
{

std::shared_ptr<BinaryData> makeBinaryData(const char* data, std::size_t size)
{
    if (data == nullptr && size > 0) {
        return nullptr;
    }

    auto binaryData = std::make_shared<BinaryData>();
    char* rawData = nullptr;

    if (size > 0) {
        rawData = new char[size];
        std::memcpy(rawData, data, size);
    }

    binaryData->assign(rawData, size, true);
    return binaryData;
}

bool writePayload(const std::shared_ptr<FileHolder>& file, const char* data, std::size_t size)
{
    if (file == nullptr || (data == nullptr && size > 0) || size > std::numeric_limits<unsigned>::max()) {
        return false;
    }

    if (!file->openFile()) {
        return false;
    }

    bool success = true;
    if (size > 0) {
        success = file->write(data, static_cast<unsigned>(size));
    }
    file->closeFile();
    return success;
}

} //Utils
} //STI
