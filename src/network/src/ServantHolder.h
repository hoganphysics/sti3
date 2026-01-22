#ifndef STI_TNETWORK_SERVANTHOLDER_H
#define STI_TNETWORK_SERVANTHOLDER_H

#include <utility>
#include "ORBManager.h"

namespace STI
{
namespace TNetwork
{

// ServantHolder<> is a RAII wrapper for CORBA servants that handles activation
// and deactivation in the ORB's POA. It adopts a raw pointer to the servant,
// taking ownership and calling _remove_ref() when done.
// Example usage:
// For IDL interface MyIface, with servant implementation MyIface_i:
//   ServantHolder<MyIface_i, MyIface> servantHolder(new MyIface_i(...));
//   MyIface::_var_type myIfaceRef = servantHolder.getRef();
// Note: The servant implementation class (MyIface_i) must derive from
// PortableServer::ServantBase (directly or indirectly) for this to work.

template <class Impl, class Iface>
class ServantHolder
{
public:

    ServantHolder() = default;
    explicit ServantHolder(Impl* servant) { reset(servant); }

    ServantHolder(const ServantHolder&) = delete;
    ServantHolder& operator=(const ServantHolder&) = delete;

    ServantHolder(ServantHolder&& other) noexcept
    : servant_(other.servant_), poa_(other.poa_._retn()), oid_(other.oid_._retn()), active_(other.active_)
    {
        other.servant_ = nullptr;
        other.active_ = false;
    }

    ServantHolder& operator=(ServantHolder&& other) noexcept
    {
        if (this != &other) {
            reset(nullptr); // release current, if any
            servant_ = other.servant_;
            poa_ = other.poa_._retn();
            oid_ = other.oid_._retn();
            active_ = other.active_;

            other.servant_ = nullptr;
            other.active_ = false;
        }
        return *this;
    }

    ~ServantHolder() { reset(nullptr); }

    void reset(Impl* servant)
    {
        if (servant_) {
            deactivate_noexcept();
            servant_->_remove_ref();   // drop adopted ref
        }

        servant_ = servant;
        active_ = false;
        poa_ = PortableServer::POA::_nil();
        oid_ = new PortableServer::ObjectId;  // allocate empty
        oid_->length(0);

        if (servant_) {
            activate_or_throw();
        }
    }

    Impl* get() const noexcept { return servant_; }
    bool active() const noexcept { return active_; }

    explicit operator bool() const noexcept { return servant_ != nullptr; }

    typename Iface::_var_type getRef() const
    {
        if (!active_ || CORBA::is_nil(poa_)) {
            return Iface::_nil();
        }
        CORBA::Object_var obj = poa_->id_to_reference(oid_);
        return Iface::_narrow(obj);
    }

    template <class... Args>
    void emplace(Args&&... args)
    {
        reset(new Impl(std::forward<Args>(args)...));
    }

private:
    
    void activate_or_throw()
    {
        auto orbManager = STI::Network::ORBManager::ORBManager::getInstance();

        if (orbManager == nullptr) {
            return;
        }

        if (!orbManager->running()) {
            return;
        }

        if (!orbManager->isPOAactive()) {
            throw CORBA::BAD_INV_ORDER();
        }

        poa_ = orbManager->getPOA();

        oid_ = poa_->activate_object(servant_); // adopts ObjectId*
        active_ = true;
    }

    void deactivate_noexcept() noexcept
    {
        if (!active_ || CORBA::is_nil(poa_)) return;

        try {
            poa_->deactivate_object(oid_);
        }
        catch (const PortableServer::POA::ObjectNotActive&) {
            // already deactivated
        }
        catch (const PortableServer::POA::WrongAdapter&) {
            // oid from different POA; indicates a logic bug
        }
        catch (const CORBA::SystemException&) {
            // log if desired
        }

        active_ = false;
    }

    Impl* servant_ = nullptr;
    PortableServer::POA_var poa_;
    PortableServer::ObjectId_var oid_;
    bool active_ = false;
};


} //TNetwork
} //STI

#endif

