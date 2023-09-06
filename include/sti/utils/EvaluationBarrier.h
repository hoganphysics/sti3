#ifndef STI_UTILS_EVALUATIONBARRIER_H
#define STI_UTILS_EVALUATIONBARRIER_H

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>


namespace STI
{
namespace Utils
{

/// The EvaluationBarrier collects multiple redundant calls to the same std::function
/// within some specified time interval and calls the std::function once at the end.


class EvaluationBarrier
{
public:

    // BarrierEvaluator
    // DelayedSingleEvaluator
    // DelayedEvaluationBarrier
    // EvaluationBarrier

    EvaluationBarrier() : waiting(false) {}
    ~EvaluationBarrier() 
    {
        join();
    }
    
    void wait(int waitTime_ms, const std::function<void(void)>& action)
    {
        return wait(std::chrono::milliseconds(waitTime_ms), action);
    }

 
    void wait(const std::chrono::duration<float>& waitTime, const std::function<void(void)>& action)
    {
        std::unique_lock<std::mutex> barrierLock(barrierMutex);

        if (waiting) return;    //already triggered

        waiting = true;
        
        auto delayedAction = [this, waitTime, action](){
            std::unique_lock<std::mutex> barrierLock(barrierMutex);

            condition.wait_for(barrierLock, waitTime);
            waiting = false;

            action();
        };
        join();
        waitingThread = std::thread(delayedAction);
    }

    void join()
    {
        if (waitingThread.joinable()) {
            waitingThread.join();
        }
    }

private:

    bool waiting;
	mutable std::mutex barrierMutex;
	mutable std::condition_variable condition;
    std::thread waitingThread;
};


} // UTILS
} // STI

#endif
