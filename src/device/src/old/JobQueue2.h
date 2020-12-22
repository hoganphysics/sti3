#ifndef STI_UTILS_JOBQUEUE_H
#define STI_UTILS_JOBQUEUE_H

#include <memory>
#include <vector>
#include <set>
#include <mutex>
#include <algorithm>


namespace STI
{
namespace Utils
{


template<typename Job, typename Compare>
class JobQueue
{
public:

	JobQueue();
	virtual ~JobQueue() {}

	void add(const Job& job);
	void remove(const Job& job);

    bool popJob(Job& topJob);                   ///Remove top job from queue and return reference
	bool peekJob(Job& topJob);		            ///Get the top job, leaving it in place

    unsigned size() const;
    bool contains(const Job& job) const;

	void getJobs(std::vector<Job>& jobs) const;

	void clear();

private:

    typename std::set<Job>::iterator _findJob(const Job& job);

	std::set<Job, Compare> jobQueue;

    mutable std::mutex queueMutex;
};


} // UTILS
} // STI


template<typename Job, typename Compare>
typename std::set<Job>::iterator STI::Utils::JobQueue<Job, Compare>::_findJob(const Job& job)
{
    auto it = std::find_if (jobQueue.begin(), jobQueue.end(), 
                            [](const Job& j)-> bool { return j == job; });
    return it;
}


template<typename Job, typename Compare>
void STI::Utils::JobQueue<Job, Compare>::add(const Job& job)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(job);
    
	if(it != jobQueue.end()) {
		jobQueue.erase(it);
	}

	jobQueue.insert(job);

    // if(it == jobQueue.end()) {
    //     jobQueue.insert(job);
    // }
}

template<typename Job, typename Compare>
void STI::Utils::JobQueue<Job, Compare>::remove(const Job& job)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);

    if(it != jobQueue.end()) {
        jobQueue.erase(it);
    }
}


///Remove top job from queue and return reference
template<typename Job, typename Compare>
bool STI::Utils::JobQueue<Job, Compare>::popJob(Job& topJob)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    if(jobQueue.size() > 0) {
        auto it = jobQueue.begin();
        topJob = *it;
        jobQueue.erase(it);
        return true;
    }
    return false;
}

///Get the top job, leaving it in place
template<typename Job, typename Compare>
bool STI::Utils::JobQueue<Job, Compare>::peekJob(Job& topJob)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    if(jobQueue.size() > 0) {
        auto it = jobQueue.begin();
        topJob = *it;
        return true;
    }
    return false;
}

template<typename Job, typename Compare>
unsigned STI::Utils::JobQueue<Job, Compare>::size() const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    return jobQueue.size();
}

template< typename Job, typename Compare>
bool STI::Utils::JobQueue<Job, Compare>::contains(const Job& job) const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(job);
   
    return it != jobQueue.end();
}

template<typename Job, typename Compare>
void STI::Utils::JobQueue<Job, Compare>::getJobs(std::vector<Job>& jobs) const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    for(auto& job : jobQueue) {
        jobs.push_back(job);
    }
}

template<typename Job, typename Compare>
void STI::Utils::JobQueue<Job, Compare>::clear()
{
    std::unique_lock<std::mutex> writeLock(queueMutex);
    jobQueue.clear();
}

#endif
