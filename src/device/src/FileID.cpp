
#include <sti/utils/FileID.h>
#include <sti/utils/utils.h>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

#include <filesystem>

namespace fs = std::filesystem;

using STI::Utils::FileID;


FileID::FileID()
{
}

std::string FileID::getFullFilename() const
{
    fs::path filePath = path;
    filePath /= filename;

    return filePath.string();
}

bool FileID::operator<(const FileID& rhs) const 
{
    if (origin != rhs.origin) return origin < rhs.origin;
    if (persistenceLocation != rhs.persistenceLocation) return persistenceLocation < rhs.persistenceLocation;
    if (path != rhs.path) return path < rhs.path;
    if (filename != rhs.filename) return filename < rhs.filename;

    return false;	//equal
}

bool FileID::operator==(const FileID& rhs) const
{
    return origin == rhs.origin 
            && persistenceLocation == rhs.persistenceLocation
            && path == rhs.path
            && filename == rhs.filename;
}

bool FileID::operator!=(const FileID& rhs) const
{
    return !((*this) == rhs);
}

std::string FileID::print() const
{
	std::stringstream fid;

    // <origin=localhost/0/Frame1, file=/data/2023/8/12/images/frame1_11_05_53.tif>
    fid << "<origin=" << origin << ", ";
    fid << "file=" << getFullFilename() << ">";

    return fid.str();
}

/// Find the deepest common path of a group of files
std::filesystem::path FileID::commonBasePath(const std::vector<FileID>& fileIDs)
{
    std::filesystem::path commonBase;

    if (fileIDs.size() == 0) return commonBase;

    commonBase = fileIDs.at(0).path;

    for (unsigned i = 1; i < fileIDs.size(); ++i) {
        commonBase = STI::Utils::findCommonBase(commonBase, fileIDs.at(i).path);
    }
    return commonBase;
}

template<class Archive>
void FileID::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("filename", filename), 
		cereal::make_nvp("path", path), 
		cereal::make_nvp("origin", origin),
        cereal::make_nvp("persistenceLocation", persistenceLocation),
        cereal::make_nvp("creationTime", creationTime)
		);
}

template void FileID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void FileID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void FileID::serialize<cereal::JSONOutputArchive>( cereal::JSONOutputArchive& );
template void FileID::serialize<cereal::JSONInputArchive>( cereal::JSONInputArchive& );


