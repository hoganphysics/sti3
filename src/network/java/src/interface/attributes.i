

%{
    #include "JAttributeManager.h"
    #include "Attribute.h"
    using STI::Device::Attribute;
%}

%shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::Attribute);


//Attributes
%template(StringMap) std::map< std::string, std::string >;
%template(StringVector) std::vector< std::string >;
%include "Attribute.h"
%template(AttributeVector) std::vector< std::shared_ptr < STI::Device::Attribute > >;
%ignore STI::Device::AttributeManager;
%ignore STI::Device::JAttributeManager::JAttributeManager(std::shared_ptr< STI::Device::AttributeManager >& manager);
%include "JAttributeManager.h"

