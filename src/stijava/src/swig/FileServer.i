
%{
    #include <sti/utils/FileServer.h>
    using STI::Utils::FileServer;
    using STI::Utils::FileTransferType;

    #include <sti/utils/VirtualFileServer.h>
    using STI::Utils::VirtualFileServer;
%}

%shared_ptr(STI::Utils::FileServer);
%shared_ptr(STI::Utils::VirtualFileServer);


//FileServer
%include "sti/utils/FileServer.h"

//VirtualFileServer
%include "sti/utils/VirtualFileServer.h"

