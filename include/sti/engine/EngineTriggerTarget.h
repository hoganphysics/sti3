#ifndef STI_ENGINE_ENGINETRIGGERTARGET_H
#define STI_ENGINE_ENGINETRIGGERTARGET_H

#include <string>

namespace STI
{
namespace Engine
{

class EngineID;
class ParseID;

class EngineTriggerTarget
{
public:

	virtual ~EngineTriggerTarget() {}

	virtual void requestTrigger(const EngineID& engineID, const ParseID& parseID) = 0;
	virtual void cancelTrigger() = 0;
};


} //Engine
} //STI

#endif
