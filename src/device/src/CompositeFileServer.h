#ifndef STI_DEVICE_COMPOSITEFILESERVER_H
#define STI_DEVICE_COMPOSITEFILESERVER_H

#include <sti/utils/FileServer.h>

#include <memory>
#include <vector>

namespace STI
{
namespace Device
{

class CompositeFileServer : public STI::Utils::FileServer
{
public:
    explicit CompositeFileServer(std::vector<std::shared_ptr<STI::Utils::FileServer>> servers);

    bool addFile(const std::shared_ptr<STI::Utils::FileHolder>& file) override;
    bool findFile(const STI::Utils::FileID& fileID) override;
    int getFileSize(const STI::Utils::FileID& fileID) override;
    bool transferFile(const STI::Utils::FileID& source,
        const std::shared_ptr<STI::Utils::FileHolder>& destination,
        STI::Utils::FileTransferType type) override;
    bool transferFilePartial(const STI::Utils::FileID& source,
        const std::shared_ptr<STI::Utils::FileHolder>& destination,
        int offset,
        int lines) override;
    bool deleteFile(const STI::Utils::FileID& fileID) override;

private:
    std::shared_ptr<STI::Utils::FileServer> findServer(const STI::Utils::FileID& fileID);

    std::vector<std::shared_ptr<STI::Utils::FileServer>> servers;
};

std::shared_ptr<STI::Utils::FileServer> makeMeasurementSourceFileServer(
    const std::shared_ptr<STI::Utils::FileServer>& measurementFileServer,
    const std::shared_ptr<STI::Utils::FileServer>& deviceFileServer);

} //Device
} //STI

#endif
