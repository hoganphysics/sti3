#ifndef STI_TNETWORK_TREFERENCEHOLDER_H
#define STI_TNETWORK_TREFERENCEHOLDER_H

#include "deviceNet.h"

#include <mutex>
#include <vector>
#include <memory>

namespace STI
{
namespace TNetwork
{

/// Holds the CORBA var reference associated with type T.
/// Takes care of disabling the reference (managing reference count) when servant needs to shut down.

class TReferenceHolderInterface
{
public:

    virtual ~TReferenceHolderInterface() {}

    virtual bool isDisabled() const = 0;
    virtual void disable() = 0;
};


template<typename T>
class TReferenceHolder : public TReferenceHolderInterface
{
public:

	TReferenceHolder(typename T::_ptr_type t_ptr, std::mutex& refMutex)
	: tReference(T::_duplicate(t_ptr)), refMutex(refMutex) {}

	virtual ~TReferenceHolder() 
    {
        disable();
    }

	void addDependent(const typename std::shared_ptr<TReferenceHolderInterface>& holder)
	{
		holders.push_back(holder);
	}

	void clean()
	{
		for(auto it = holders.begin(); it != holders.end();) {
			if ((*it) != 0 && (*it)->isDisabled()) {
				it = holders.erase(it);
			}
			else {
				++it;
			}
		}
	}

	bool isDisabled() const
	{
		return CORBA::is_nil(tReference);
	}

	void disable()
	{
		std::unique_lock<std::mutex> refLock(refMutex);
		
		disable(refLock);
	}

	void disable(const std::unique_lock<std::mutex>& lock)
	{
		typename T::_var_type nilRef = T::_nil();
		tReference = nilRef;	//release reference; reference is now nil

		for(auto& holder : holders) {
			if (holder != 0) {
				holder->disable();
			}
		}
		clean();
	}

	typename T::_var_type& getTRef()
	{
		return tReference;
	}

    const typename T::_var_type& getTRef() const
	{
		return tReference;
	}

private:

	typename T::_var_type tReference;

	std::vector<std::shared_ptr<TReferenceHolderInterface>> holders;

	std::mutex& refMutex;
};


} //TNetwork
} //STI


#endif

