#include <sti/utils/FileBinaryDataStream.h>

#include <sti/utils/BinaryData.h>
#include <sti/utils/FileHolder.h>

#include <openssl/evp.h>

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace
{

class BinaryDataStreamFileHolder : public STI::Utils::FileHolder
{
public:
    BinaryDataStreamFileHolder(
        const STI::Utils::FileID& sourceID,
        const std::shared_ptr<STI::Utils::BinaryDataStreamTarget>& target,
        std::size_t chunkSize)
        : fileID(sourceID),
          target(target),
          chunkSize((std::max)(std::size_t{1}, chunkSize))
    {
    }

    ~BinaryDataStreamFileHolder() override
    {
        if (digestContext != nullptr) {
            EVP_MD_CTX_free(digestContext);
        }
    }

    STI::Utils::FileID getID() const override { return fileID; }
    std::string getFilename() const override { return fileID.getFullFilename(); }
    unsigned getFileSize() const override { return bytesWritten; }
    bool exists() const override { return complete; }
    bool transferFile(const std::shared_ptr<STI::Utils::FileHolder>&) override { return false; }

    unsigned maxBufferSize() const override
    {
        return static_cast<unsigned>((std::min)(
            chunkSize,
            static_cast<std::size_t>((std::numeric_limits<unsigned>::max)())));
    }

    std::string md5Checksum() override { return checksum; }

    bool write(const char* buffer, unsigned length) override
    {
        if (!open || target == nullptr || (buffer == nullptr && length > 0)) {
            return false;
        }

        if (length > 0) {
            auto data = std::make_shared<STI::Utils::BinaryData>();
            auto* copy = new char[length];
            std::memcpy(copy, buffer, length);
            data->assign(copy, length, true);
            target->writeNext(data);

            if (digestContext == nullptr || EVP_DigestUpdate(digestContext, buffer, length) != 1) {
                return false;
            }
        }

        bytesWritten += length;
        return true;
    }

    bool openFile() override
    {
        if (target == nullptr || open) {
            return false;
        }

        digestContext = EVP_MD_CTX_new();
        if (digestContext == nullptr || EVP_DigestInit_ex(digestContext, EVP_md5(), nullptr) != 1) {
            if (digestContext != nullptr) {
                EVP_MD_CTX_free(digestContext);
                digestContext = nullptr;
            }
            return false;
        }

        checksum.clear();
        bytesWritten = 0;
        complete = false;
        open = true;
        target->start();
        return true;
    }

    void closeFile() override
    {
        if (!open) {
            return;
        }

        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned digestLength = 0;
        if (digestContext != nullptr
            && EVP_DigestFinal_ex(digestContext, digest, &digestLength) == 1) {
            std::stringstream stream;
            stream << std::hex << std::uppercase << std::setfill('0');
            for (unsigned i = 0; i < digestLength; ++i) {
                stream << std::setw(2) << static_cast<int>(digest[i]);
            }
            checksum = stream.str();
        }

        if (digestContext != nullptr) {
            EVP_MD_CTX_free(digestContext);
            digestContext = nullptr;
        }

        target->stop();
        complete = true;
        open = false;
    }

private:
    STI::Utils::FileID fileID;
    std::shared_ptr<STI::Utils::BinaryDataStreamTarget> target;
    std::size_t chunkSize;
    EVP_MD_CTX* digestContext{nullptr};
    std::string checksum;
    unsigned bytesWritten{0};
    bool open{false};
    bool complete{false};
};

} // namespace

STI::Utils::FileBinaryDataStream::FileBinaryDataStream(
    const std::shared_ptr<FileHolder>& file,
    std::size_t chunkSize)
    : file(file), chunkSize(chunkSize)
{
}

STI::Utils::FileBinaryDataStream::~FileBinaryDataStream() = default;

void STI::Utils::FileBinaryDataStream::transfer(
    const std::shared_ptr<BinaryDataStreamTarget>& target)
{
    if (file == nullptr || target == nullptr) {
        return;
    }

    auto destination = std::make_shared<BinaryDataStreamFileHolder>(file->getID(), target, chunkSize);
    file->transferFile(destination);
}
