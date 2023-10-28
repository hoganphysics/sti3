
#include "VirtualFileHolder.h"


using STI::Utils::VirtualFileHolder;
using STI::Utils::FileID;


VirtualFileHolder::VirtualFileHolder(const std::string& originID, const FileID& fileID)
: LocalFileHolder(originID, fileID)
{
    data = std::make_shared<std::stringstream>();
}

VirtualFileHolder::~VirtualFileHolder()
{
}

std::ostream* VirtualFileHolder::getostream()
{
    return data.get();
}

bool VirtualFileHolder::getistream(std::shared_ptr<std::istream>& istream)
{
    if (data == 0) return false;

    // data->seekg(std::ios_base::end);
    data->seekg(0, std::ios::end);

    istream = data;
    return true;
}

bool VirtualFileHolder::openFile()
{
    data = std::make_shared<std::stringstream>();
    data->clear();
    data->str("");
    return true;
}

void VirtualFileHolder::closeFile()
{
    // std::cout << "File: " << getID().filename << std::endl;

    // std::cout << "File: " << data->str() << std::endl;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip1 fp)
{
    (*data) << fp;
    return *this;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip2 fp)
{
    (*data) << fp;
    return *this;
}

VirtualFileHolder& VirtualFileHolder::operator<<(manip3 fp)
{
    (*data) << fp;
    return *this;
}

