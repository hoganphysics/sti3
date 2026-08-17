#ifndef STI_UTILS_FILEBINARYDATASTREAM_H
#define STI_UTILS_FILEBINARYDATASTREAM_H

#include <sti/utils/BinaryDataStream.h>

#include <cstddef>
#include <memory>

namespace STI
{
namespace Utils
{

class FileHolder;

// Adapts a FileHolder to the BinaryData streaming interface without loading the
// complete file into memory. The source FileHolder remains alive for the life
// of the stream.
class FileBinaryDataStream : public BinaryDataStream
{
public:
    FileBinaryDataStream(const std::shared_ptr<FileHolder>& file, std::size_t chunkSize);
    ~FileBinaryDataStream();

    void transfer(const std::shared_ptr<BinaryDataStreamTarget>& target) override;

private:
    std::shared_ptr<FileHolder> file;
    std::size_t chunkSize;
};

} //Utils
} //STI

#endif
