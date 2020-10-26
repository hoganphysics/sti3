#ifndef STI_UTILS_JOBQUEUEGROUP_H
#define STI_UTILS_JOBQUEUEGROUP_H

#include <memory>
#include <vector>
#include <set>

namespace STI
{
namespace Utils
{


// template<typename E, typename Compare>
// class PriorityQueue
// {
// public:

// 	PriorityQueue();
// 	virtual ~PriorityQueue() {}

// 	unsigned size() const;
// 	bool push(E element);		///Add element to the queue according to Compare
// 	bool pop(E& element);		///Take the top element, removing from queue
// 	bool peek(E& element);		///Get the top element, leaving it in place
	
// //	bool contains(const ID& id) const;
// //	void remove(const ID& id);

// 	void getElements(std::vector<E>& el);	//Problem -- using this breaks the heap!  just use set...

// 	void clear();

// private:

// 	Compare compare;
// 	std::vector<E> elements;
// };


// template<typename ID, typename Job, typename Compare>
// class PriorityJobQueue : public PriorityQueue<Job, Compare>
// {
// public:

// 	PriorityJobQueue();
// 	virtual ~PriorityJobQueue() {}

// 	bool contains(const ID& id) const;
// 	void remove(const ID& id);

// };


template<typename Job>
class JobConcurrencyPolicy
{
public:
	virtual bool runConcurrent(const Job& runningJob, const Job& newJob) const = 0;
};

//class JobID { virtual bool operator<(...) = 0; }
//class ParseID : public JobID;
//class PlayID : public JobID;

//No, need to be able to compare ParseID and PlayID

//JobQueueGroup<JobID, TimingJob<ParseID>>

template<typename ID, typename Job, typename Compare, typename ConcurrencyPolicy>
class JobQueueGroup
{
public:

	//typedef std::shared_ptr<JobConcurrencyPolicy<Job>> ConcurrencyPolicy_ptr;

	JobQueueGroup(unsigned size);
	virtual ~JobQueueGroup() {}

	//void setConcurrencyPolicy(const ConcurrencyPolicy_ptr& policy) { _policy = policy; }

	void setNumberOfQueues(unsigned size);
	void numberOfQueues() const;

	void addJob(unsigned queue, Job job);
	void moveJob(const ID& jobID, unsigned newQueue);

	void recallJob(const ID& jobID);	//abort and return job to queue
	void cancelJob(const ID& jobID);	//abort and delete job

	void getRunningJobs(std::vector<Job>& jobs) const;
	void getAllJobs(unsigned queue, std::vector<Job>& jobs) const;

	void clear(unsigned queue);
	void clearAll();

private:
	
	ConcurrencyPolicy policy;
	Compare compare;
	
	//list of running jobs (right before running, a job is removed from it's queue and put in this list)

};


} // UTILS
} // STI

template<typename ID, typename Job, typename Compare, typename ConcurrencyPolicy>
STI::Utils::JobQueueGroup<ID, Job, Compare, ConcurrencyPolicy>::JobQueueGroup(unsigned size)
{
}



#endif
