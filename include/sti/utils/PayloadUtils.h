#ifndef STI_UTILS_PAYLOADUTILS_H
#define STI_UTILS_PAYLOADUTILS_H

#include <cstddef>
#include <memory>

namespace STI
{
namespace Utils
{

class BinaryData;
class FileHolder;

std::shared_ptr<BinaryData> makeBinaryData(const char* data, std::size_t size);
bool writePayload(const std::shared_ptr<FileHolder>& file, const char* data, std::size_t size);

} //Utils
} //STI

#endif
