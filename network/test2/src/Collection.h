#ifndef STI_UTILS_COLLECTION_H
#define STI_UTILS_COLLECTION_H

#include <set>
#include <memory>


namespace STI
{
namespace Utils
{


template<class ID, class T>
class Collection
{
protected:
	typedef std::shared_ptr<T> T_ptr;

public:

	virtual ~Collection() {}

	virtual bool add(const ID& id, const T_ptr& node) = 0;
	virtual bool remove(const ID& id) = 0;

	virtual bool contains(const ID& id) const = 0;

	virtual bool get(const ID& id, T_ptr& node) const = 0;
	virtual void getIDs(std::set<ID>& ids) const = 0;

//	virtual void cleanup(const std::set<ID>& ids) = 0;	//Remove any collected nodes that are NOT in ids
	virtual void cleanup() = 0;							//Remove any collected nodes that violate policy

	virtual void clear() = 0;
};


} //Utils
} //STI

#endif
