#ifndef STI_DEVICE_JSHOT_H
#define STI_DEVICE_JSHOT_H

#include <sti/engine/Shot.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/engine/ShotConfig.h>

#include <memory>


namespace STI
{
namespace Engine
{


//Java Shot wrapper
class JShot : public Shot
{
public:
	
	JShot(std::shared_ptr<STI::Engine::Shot>& shot);
	~JShot();

    const ShotConfig& getShotConfig() const;
    std::shared_ptr<RawEventGroup> getRootEventGroup();

private:

    void getRootEventGroup(std::shared_ptr<RawEventGroup>& rootGroup);

    ShotConfig shotConfig;
    std::shared_ptr<STI::Engine::Shot> shot_;

};

} //Device
} //STI

#endif
