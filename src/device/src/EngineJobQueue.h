#ifndef STI_ENGINE_ENGINEJOBQUEUE_H
#define STI_ENGINE_ENGINEJOBQUEUE_H

#include "utils/JobQueue.h"
#include "EngineJobID.h"

#include <memory>
#include <thread>

namespace STI
{
namespace Engine
{

typedef STI::Utils::Job<EngineJobID> EngineJob;

// class EngineJob : public STI::Utils::Job<EngineJobID>
// {
// public:

// 	EngineJobID id() const;
// 	void run();
// 	void abort();
//     bool running();

// };

class EngineJobCompare
{
public:
    bool operator()(const std::shared_ptr<EngineJob>& lhs, const std::shared_ptr<EngineJob>& rhs)
    {
        return (lhs != 0 && rhs != 0) && (lhs->id() < rhs->id());
    }
};

class EngineJobQueue : public STI::Utils::JobQueue<EngineJobID, std::shared_ptr<EngineJob>, EngineJobCompare>
{
public:

    EngineJobQueue();

    bool runJob();
    void cancelJob();
    bool running() const;
    bool getRunningJob(std::shared_ptr<EngineJob>& job);


private:
	
    void start();
	void stop();

	std::thread eventThread;
	bool running;

};


} //Engine
} //STI

#endif
