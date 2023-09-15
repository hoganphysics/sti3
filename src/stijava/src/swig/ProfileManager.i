%{
    #include "JProfileManager.h"
    using STI::Device::JProfileManager;

    #include <sti/device/Profile.h>
    using STI::Device::Profile;
    using STI::Device::ProfileType;
%}

%ignore STI::Device::Profiles;

%shared_ptr(STI::Device::Profile);

%include "sti/device/Profile.h"

%ignore STI::Device::ProfileManager;
%ignore STI::Device::JProfileManager::JProfileManager(const std::shared_ptr< STI::Device::ProfileManager >& manager);
%include "JProfileManager.h"
