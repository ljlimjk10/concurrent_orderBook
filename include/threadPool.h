#pragma once
#include <atomic>
#include <thread>
#include "queue.h"

class JoinThreads
{
public:
	explicit JoinThreads(std::vector<std::thread>& threads)
		: threads_{threads}
	{};

	~JoinThreads()
	{
		for (size_t t=0; t<threads_.size(); ++t)
		{
			if (threads_[t].joinable())
			{
				threads_[t].join();
			}
		}
	}
private:
	std::vector<std::thread>& threads_;
};

class ThreadPool
{
public:
	ThreadPool()
		: done_{false}
	, joiner_{threads_}
	{
		uint const supportedConcurrentThreads = std::thread::hardware_concurrency();
		try
		{
			for (uint i=0; i<supportedConcurrentThreads; ++i)
			{
				threads_.push_back(std::thread{&ThreadPool::runWorkerThread, this});
			}
		}
		catch (...)
		{
			done_ = true;
			throw;
		}
	}

	~ThreadPool()
	{
		done_ = true;
	}

	template<typename Function>
	void submit(Function&& f)
	{
		workQueue_.push(std::function<void()>{std::forward<Function>(f)});
	}



private:
	std::atomic_bool done_;
	ThreadSafeQueue<std::function<void()>> workQueue_;
	std::vector<std::thread> threads_;
	JoinThreads joiner_;

	void runWorkerThread()
	{
		while (!done_)
		{
			std::function<void()> currTask;
			if (workQueue_.tryPop())
			{
				currTask();
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

};
