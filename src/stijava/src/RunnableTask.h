#ifndef STI_DEVICE_RUNNABLETASK_H
#define STI_DEVICE_RUNNABLETASK_H

#include <string>

namespace STI
{
namespace Device
{

class RunnableTask
{
public:
	
	RunnableTask() {}
    virtual ~RunnableTask() {}

    virtual void run() { }

};

} //Device
} //STI

#endif
