%feature("director");

%{
    #include "DeviceID.h"
    #include "Device.h"
    #include "JDevice.h"
    #include "DeviceCollection.h"
    #include "JDeviceCollection.h"
    #include "JNetworkDeviceHub.h"
%}

%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"

%shared_ptr(STI::Device::JDevice);
%shared_ptr(STI::Device::DeviceCollection);
%shared_ptr(STI::Device::JDeviceCollection);

//DeviceID
%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;
%include "DeviceID.h"
%template(DeviceIDset) std::set< STI::Device::DeviceID >;


//JDeviceCollection
%ignore STI::Device::DeviceCollection;
%include "JDeviceCollection.h"


//JDevice
%ignore STI::Device::Device;
%include "JDevice.h"

//JNetworkDeviceHub
%include "JNetworkDeviceHub.h"
