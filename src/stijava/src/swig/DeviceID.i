
%{
    #include <sti/device/DeviceID.h>
%}


//DeviceID

%ignore DeviceIDBase;
%include "sti/device/DeviceID.h"
%template(DeviceIDset) std::set< STI::Device::DeviceID >;
%template(DeviceIDvector) std::vector< STI::Device::DeviceID >;

