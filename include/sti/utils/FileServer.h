#ifndef STI_UTILS_FILESERVER_H
#define STI_UTILS_FILESERVER_H

#include <sti/utils/FileID.h>
#include <sti/utils/FileHolder.h>

#include <memory>
#include <string>


namespace STI
{
namespace Utils
{

enum class FileTransferType { Binary, String };


class FileServer
{
public:

	virtual ~FileServer() {}

	virtual bool addFile(const std::shared_ptr<FileHolder>& file) = 0;
	virtual bool findFile(const FileID& fileID) = 0;
	virtual int getFileSize(const FileID& fileID) = 0;
	virtual bool transferFile(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, FileTransferType type) = 0;
	virtual bool transferFilePartial(const FileID& source, const std::shared_ptr<STI::Utils::FileHolder>& destination, int offset, int lines) = 0;
	virtual bool deleteFile(const FileID& fileID) = 0;

};


} //Utils
} //STI

#endif
