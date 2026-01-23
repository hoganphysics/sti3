#include "TFileHolder_i.h"

#include "NetworkConvert.h"
#include "RemoteFileHolder.h"
#include "convert/Convert_File.h"


using STI::TNetwork::TFileHolder_i;
using STI::Network::convertBuffer;
using STI::Network::convert;
using STI::TNetwork::TFileID;


TFileHolder_i::TFileHolder_i(STI::Utils::FileHolder* fileHolder)
: localFileHolder(fileHolder)
{
}

TFileHolder_i::~TFileHolder_i()
{
}

TFileID* TFileHolder_i::getID()
{
	STI::TNetwork::TFileID_var tFileID(new STI::TNetwork::TFileID);

	if(localFileHolder != 0) {
        auto fileID = localFileHolder->getID();
		convert<STI::Utils::FileID, TFileID>(fileID, tFileID);
	}

	return tFileID._retn();
}

char* TFileHolder_i::getFilename()
{
    std::string result = "";

    if (localFileHolder != 0) {
		result = localFileHolder->getFilename();
	}

    CORBA::String_var tResult = convert<std::string, CORBA::String_member>(result);
    return tResult._retn();
}

::CORBA::Boolean TFileHolder_i::exists()
{
    bool result = false;

    if (localFileHolder != 0) {
		result = localFileHolder->exists();
	}
    return static_cast<::CORBA::Boolean>(result);
}

char* TFileHolder_i::md5Checksum()
{
    std::string result = "";

    if (localFileHolder != 0) {
		result = localFileHolder->md5Checksum();
	}

    CORBA::String_var tResult = convert<std::string, CORBA::String_member>(result);
    return tResult._retn();
}

::CORBA::Boolean TFileHolder_i::transferFile(::STI::TNetwork::TFileHolder_ptr destination)
{
    bool result = false;

    if (localFileHolder != 0 && !CORBA::is_nil(destination)) {
        STI::TNetwork::TFileHolder_var destination_var = STI::TNetwork::TFileHolder::_duplicate(destination);
        auto remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(destination_var);
		result = localFileHolder->transferFile(remoteFile);
	}
    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Long TFileHolder_i::maxBufferSize()
{
    int result = 32*1024;   //default

    if (localFileHolder != 0) {
		result = localFileHolder->maxBufferSize();
	}
    return static_cast<::CORBA::Long>(result);
}

::CORBA::Boolean TFileHolder_i::write(const ::STI::TNetwork::OctetSeq& buffer)
{
    bool result = false;

    if (localFileHolder != 0) {
        unsigned char* data = const_cast<STI::TNetwork::OctetSeq&>(buffer).get_buffer();    //no deep copy
        char* dataC = reinterpret_cast<char*>(data);
        result = localFileHolder->write(dataC, buffer.length());
	}
    return static_cast<::CORBA::Boolean>(result);
}


::CORBA::Boolean TFileHolder_i::openFile()
{
    bool result = false;

    if (localFileHolder != 0) {
		result = localFileHolder->openFile();
	}
    return static_cast<::CORBA::Boolean>(result);
}

void TFileHolder_i::closeFile()
{
    if (localFileHolder != 0) {
		localFileHolder->closeFile();
	}
}
