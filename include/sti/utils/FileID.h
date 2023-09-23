#ifndef STI_UTILS_FILEID_H
#define STI_UTILS_FILEID_H

#include <sti/utils/TimeStamp.h>
#include <string>
#include <vector>
#include <filesystem>

namespace STI
{
namespace Utils
{


class FileID
{
public:

	FileID();
	
	std::string getFullFilename() const;

    std::string filename;
    std::string path;
    std::string origin;		//where file was created
    std::string persistenceLocation;
    TimeStamp creationTime;

	bool operator<(const FileID& rhs) const;
	bool operator==(const FileID& rhs) const;
	bool operator!=(const FileID& rhs) const;

	std::string print() const;

	static std::filesystem::path commonBasePath(const std::vector<FileID>& fileIDs);

	template<class Archive>
	void serialize(Archive& archive);

};


} //Utils
} //STI

#endif

