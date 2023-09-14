#ifndef STI_UTILS_VIRTUALFILESERVER_H
#define STI_UTILS_VIRTUALFILESERVER_H

#include <sti/utils/FileServer.h>
#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>
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

    void addFile(const std::shared_ptr<FileHolder>& destination);

	bool findFile(const FileID& fileID);
	int getFileSize(const FileID& fileID);
	bool transferFile(const FileID& source, const std::shared_ptr<FileHolder>& destination, FileTransferType type);
	bool transferFilePartial(const FileID& source, const std::shared_ptr<FileHolder>& destination, int offset, int lines);
	bool deleteFile(const FileID& fileID);

private:

    STI::Utils::SynchronizedMap<FileID, std::shared_ptr<FileHolder>> files;

};


} //Utils
} //STI

#endif

