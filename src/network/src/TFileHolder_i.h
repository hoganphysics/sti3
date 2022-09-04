#ifndef STI_TNETWORK_TFILEHOLDER_I_H
#define STI_TNETWORK_TFILEHOLDER_I_H

#include "deviceNet.h"

#include <sti/utils/FileHolder.h>


namespace STI
{
namespace TNetwork
{

class TFileHolder_i : public POA_STI::TNetwork::TFileHolder
{
public:

	TFileHolder_i(STI::Utils::FileHolder* fileHolder);
	~TFileHolder_i();
	
    char* getFilename();
    ::CORBA::Boolean exists();
    char* md5Checksum();
    ::CORBA::Boolean transferFile(::STI::TNetwork::TFileHolder_ptr destination);
    ::CORBA::Long maxBufferSize();
    ::CORBA::Boolean deleteFile();
    ::CORBA::Boolean write(const ::STI::TNetwork::OctetSeq& buffer);
    ::CORBA::Boolean openFile();
    void closeFile();

private:

    STI::Utils::FileHolder* localFileHolder;
};


} //TNetwork
} //STI


#endif

