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


template<typename ID>
class Job
{
public:

	virtual ID id() const = 0;
	virtual void run() = 0;
	virtual void abort() = 0;
    virtual bool running() = 0;

};



template<typename ID, typename Job, typename Compare>
class JobQueue	//JobQueueRunner?
{
public:

	JobQueue();
	virtual ~JobQueue() {}

	void addJob(Job job);
	void cancelJob(const ID& jobID);
    
    bool pullJob(const ID& jobID, Job& job);	///Remove from job from queue and return by reference
    bool popJob(Job& topJob);                   ///Remove top job from queue and return reference
	bool peekJob(Job& topJob);		            ///Get the top job, leaving it in place

    unsigned size() const;
    bool contains(const ID& jobID) const;

	bool getJob(const ID& jobID, Job& job) const;
	void getJobs(std::vector<ID>& ids) const;

	void remove(const ID& jobID);
	void clear();

private:

    typename std::set<Job>::iterator _findJob(const ID& jobID);

//	virtual void runJob(Job job) = 0;

//	PriorityJobQueue<ID, Job, Compare> jobQueue;	//maybe just use std::set with custom Compare
	std::set<Job, Compare> jobQueue;

    mutable std::mutex queueMutex;
};


} // UTILS
} // STI


//Implementation

template<typename ID, typename Job, typename Compare>
STI::Utils::JobQueue<ID, Job, Compare>::JobQueue()
{
}


template<typename ID, typename Job, typename Compare>
typename std::set<Job>::iterator STI::Utils::JobQueue<ID, Job, Compare>::_findJob(const ID& jobID)
{
    auto it = std::find_if (jobQueue.begin(), jobQueue.end(), 
                            [](const Job& job)-> bool { return job.id() == jobID; });
    return it;
}

template<typename ID, typename Job, typename Compare>
void STI::Utils::JobQueue<ID, Job, Compare>::addJob(Job job)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);
    
    if(it == jobQueue.end() || !it->running()) {
        jobQueue.insert(job);
    }
}

template<typename ID, typename Job, typename Compare>
void STI::Utils::JobQueue<ID, Job, Compare>::cancelJob(const ID& jobID)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);

    if(it != jobQueue.end()) {
        it->abort();
        erase(it);
    }
}

template<typename ID, typename Job, typename Compare>
bool STI::Utils::JobQueue<ID, Job, Compare>::pullJob(const ID& jobID, Job& job)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);

    if(it != jobQueue.end()) {
        job = *it;
        jobQueue.erase(it);

        return true;
    }
    return false;
}

///Remove top job from queue and return reference
template<typename ID, typename Job, typename Compare>
bool STI::Utils::JobQueue<ID, Job, Compare>::popJob(Job& topJob)
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
template<typename ID, typename Job, typename Compare>
bool STI::Utils::JobQueue<ID, Job, Compare>::peekJob(Job& topJob)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    if(jobQueue.size() > 0) {
        auto it = jobQueue.begin();
        topJob = *it;
        return true;
    }
    return false;
}

template<typename ID, typename Job, typename Compare>
unsigned STI::Utils::JobQueue<ID, Job, Compare>::size() const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    return jobQueue.size();
}

template<typename ID, typename Job, typename Compare>
bool STI::Utils::JobQueue<ID, Job, Compare>::contains(const ID& jobID) const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);
   
    return it != jobQueue.end();
}

template<typename ID, typename Job, typename Compare>
bool STI::Utils::JobQueue<ID, Job, Compare>::getJob(const ID& jobID, Job& job) const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);

    if(it != jobQueue.end()) {
        job = *it;
        return true;
    }

    return false;
}

template<typename ID, typename Job, typename Compare>
void STI::Utils::JobQueue<ID, Job, Compare>::getJobs(std::vector<ID>& ids) const
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    for(auto& job : jobQueue) {
        ids.push_back(job.id());
    }
}

template<typename ID, typename Job, typename Compare>
void STI::Utils::JobQueue<ID, Job, Compare>::remove(const ID& jobID)
{
    std::unique_lock<std::mutex> writeLock(queueMutex);

    auto it = _findJob(jobID);

    if(it != jobQueue.end()) {
        jobQueue.erase(it);
    }
}

template<typename ID, typename Job, typename Compare>
void STI::Utils::JobQueue<ID, Job, Compare>::clear()
{
    std::unique_lock<std::mutex> writeLock(queueMutex);
    jobQueue.clear();
}


#endif
