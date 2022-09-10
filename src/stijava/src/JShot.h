#ifndef STI_DEVICE_JSHOT_H
#define STI_DEVICE_JSHOT_H

#include <sti/engine/Shot.h>
#include <sti/fwd/RawEvent_fwd.h>

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
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> getEvents();

private:

    void getEvents(std::shared_ptr<std::vector<RawEvent>>& evts);

    const ShotConfig& shotConfig;
    std::shared_ptr<STI::Engine::Shot> shot_;

};

} //Device
} //STI

#endif
