
#ifndef STI_PYTHON_DEVICECOLLECTIONPY_H
#define STI_PYTHON_DEVICECOLLECTIONPY_H

#include "DeviceCollection.h"
#include "DeviceID.h"

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

class DevicePy2;

class DeviceCollectionPy
{
public:

    DeviceCollectionPy(const std::shared_ptr<STI::Device::DeviceCollection>& collection);
    virtual ~DeviceCollectionPy();

	bool add(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Python::DevicePy2>& node);
	bool remove(const STI::Device::DeviceID& id);

	bool contains(const STI::Device::DeviceID& id) const;
	unsigned size() const;

	std::shared_ptr<STI::Python::DevicePy2> get(const STI::Device::DeviceID& id) const;
	std::vector<STI::Device::DeviceID> getIDs() const;

	void clear();


private:

    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;

};


} //Python
} //STI

#endif

