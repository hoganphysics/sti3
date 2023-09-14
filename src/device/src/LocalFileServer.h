#ifndef STI_UTILS_LOCALFILESERVER_H
#define STI_UTILS_LOCALFILESERVER_H

#include <sti/utils/FileServer.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/LocalFileHolder.h>

#include <sti/device/DeviceID.h>

#include <memory>
#include <string>


namespace STI
{
namespace Utils
{


class LocalFileServer : public STI::Utils::FileServer
{
public:

    LocalFileServer(const STI::Device::DeviceID& localID);
	~LocalFileServer();

	bool findFile(const FileID& fileID);
	int getFileSize(const FileID& fileID);
	bool transferFile(const FileID& source, const std::shared_ptr<FileHolder>& destination, FileTransferType type);
	bool transferFilePartial(const FileID& source, const std::shared_ptr<FileHolder>& destination, int offset, int lines);
	bool deleteFile(const FileID& fileID);

private:

    STI::Device::DeviceID localID;

    LocalFileHolderFactory localFileHolderFactory;
};


} //Utils
} //STI

#endif

