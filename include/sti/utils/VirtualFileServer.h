#ifndef STI_UTILS_VIRTUALFILESERVER_H
#define STI_UTILS_VIRTUALFILESERVER_H

#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/SynchronizedMap.h>

#include <memory>
#include <string>


namespace STI
{
namespace Utils
{


class VirtualFileServer : public STI::Utils::FileServer
{
public:

    VirtualFileServer();
	~VirtualFileServer();

    bool addFile(const std::shared_ptr<FileHolder>& destination) override;

	bool findFile(const FileID& fileID);
	int getFileSize(const FileID& fileID);
	bool transferFile(const FileID& source, const std::shared_ptr<FileHolder>& destination, FileTransferType type);
	bool transferFilePartial(const FileID& source, const std::shared_ptr<FileHolder>& destination, int offset, int lines);
	bool deleteFile(const FileID& fileID);

private:

    STI::Utils::SynchronizedMap<FileID, std::shared_ptr<FileHolder>> files;
};



class VirtualFileServerFactory
{
public:

	~VirtualFileServerFactory() {}

	virtual std::shared_ptr<VirtualFileServer> makeVirtualFileServer() = 0;
};

class LocalVirtualFileServerFactory : public VirtualFileServerFactory
{
public:

	std::shared_ptr<VirtualFileServer> makeVirtualFileServer()
	{
		auto fileServer = std::make_shared<VirtualFileServer>();
		return fileServer;
	}
};


} //Utils
} //STI

#endif
