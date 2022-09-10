#include "TFileHolder_i.h"

#include "ORBManager.h"
#include "NetworkConvert.h"
#include "RemoteFileHolder.h"

using STI::TNetwork::TFileHolder_i;
using STI::Network::convertBuffer;
using STI::Network::convert;


TFileHolder_i::TFileHolder_i(STI::Utils::FileHolder* fileHolder)
: localFileHolder(fileHolder)
{
}

TFileHolder_i::~TFileHolder_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
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

    if (localFileHolder != 0) {
        auto remoteFile = std::make_shared<STI::Network::RemoteFileHolder>(destination);
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

::CORBA::Boolean TFileHolder_i::deleteFile()
{
    bool result = false;

    if (localFileHolder != 0) {
		result = localFileHolder->deleteFile();
	}
    return static_cast<::CORBA::Boolean>(result);
}

::CORBA::Boolean TFileHolder_i::write(const ::STI::TNetwork::OctetSeq& buffer)
{
    bool result = false;

    if (localFileHolder != 0) {

        char* copyBuffer = new char[buffer.length()];

        convertBuffer(buffer, copyBuffer);

		result = localFileHolder->write(copyBuffer, buffer.length());

        delete[] copyBuffer;
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
