

%{
    #include "AttributeRefresher.h"
    using STI::Device::AttributeRefresher;
    #include "AttributeSetter.h"
    using STI::Device::AttributeSetter;

    #include "JAttributeManager.h"
    using STI::Device::JAttributeManager;
    #include <sti/device/Attribute.h>
    #include <sti/device/LocalAttribute.h>
    using STI::Device::LocalAttribute;

    #include "AttributeRefreshListener.h"
    using STI::Device::AttributeRefreshListener;
%}

%shared_ptr(STI::Device::JAttributeManager);
%shared_ptr(STI::Device::Attribute);
%shared_ptr(STI::Device::LocalAttribute);
%shared_ptr(STI::Device::AttributeRefresher);
%shared_ptr(STI::Device::AttributeSetter);




//Attributes
%template(StringMap) std::map< std::string, std::string >;
%template(AttributeVector) std::vector< std::shared_ptr < STI::Device::Attribute > >;
%ignore STI::Device::AttributeManager;
%ignore STI::Device::JAttributeManager::JAttributeManager(const std::shared_ptr< STI::Device::AttributeManager >& manager);
%include "sti/device/Attribute.h"

%include "AttributeRefresher.h"
%include "AttributeSetter.h"

%ignore STI::Device::LocalAttribute::setRefresher(const std::function< std::string( void ) >& refesher);
%ignore STI::Device::LocalAttribute::setSetter(const std::function< bool( const std::string& ) >& setter);
%include "sti/device/LocalAttribute.h"
%extend STI::Device::LocalAttribute 
{
    STI::Device::LocalAttribute& STI::Device::LocalAttribute::setRefresher(const std::shared_ptr< STI::Device::AttributeRefresher >& refesher)
    {
        self->setRefresher(
            [refesher]() { return refesher->refresh(); }
        );
        return (*self);
    }
    STI::Device::LocalAttribute& STI::Device::LocalAttribute::setSetter(const std::shared_ptr< STI::Device::AttributeSetter >& setter)
    {
        self->setSetter(
            [setter](const std::string& value) { return setter->set(value); }
        );
        return (*self);
    }
}

%include "AttributeRefreshListener.h"
%include "JAttributeManager.h"

