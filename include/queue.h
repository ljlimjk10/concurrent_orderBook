#pragma once
#include <queue>

template<class T>
class ThreadSafeQueue
{
public:
	ThreadSafeQueue() = default;

	template<class U>
	void push(U&& newVal)
	{
		std::unique_ptr<T> res{std::unique_ptr<T>{std::forward<U>(newVal)}};
		std::lock_guard<std::mutex> lock{mut_};
		dataQueue_.push(res);
		dataCondition_.notify_one();
	}

	void waitAndPop(T& val)
	{
		std::unique_lock<std::mutex> lock{mut_};
		dataCondition_.wait(lock, [this]{return !dataQueue_.empty();});
		val = std::move(*dataQueue_.front());
		dataQueue_.pop();
	}

	std::unique_ptr<T> waitAndPop()
	{
		std::unique_lock<std::mutex> lock{mut_};
		dataCondition_.wait(lock, [this]{return !dataQueue_.empty();});
		std::unique_ptr<T> res{std::move(dataQueue_.front())};
		dataQueue_.pop();
		return res;
	}

	std::unique_ptr<T> tryPop()
	{
		std::lock_guard<std::mutex> lock{mut_};
		if (dataQueue_.empty()) return std::unique_ptr<T>{};
		std::unique_ptr<T> res{std::move(dataQueue_.front())};
		dataQueue_.pop();
		return res;
	}

	bool tryPop(T& val)
	{
		std::lock_guard<std::mutex> lock{mut_};
		if (dataQueue_.empty()) return false;
		val = std::move(*dataQueue_.front());
		dataQueue_.pop();
		return true;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock{mut_};
		return dataQueue_.empty();
	}


private:
	mutable std::mutex mut_;
	std::queue<std::unique_ptr<T>> dataQueue_;
	std::condition_variable dataCondition_;
};
