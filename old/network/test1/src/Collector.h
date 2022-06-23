#ifndef STI_UTILS_COLLECTOR_H
#define STI_UTILS_COLLECTOR_H

#include <sti/utils/Collection.h>

#include <memory>

namespace STI
{
namespace Utils
{

template<class ID, class T>
class Collector
{
public:

	virtual void getCollection(std::shared_ptr<STI::Utils::Collection<ID, T>>& collection) = 0;
};


} //Utils
} //STI

#endif
