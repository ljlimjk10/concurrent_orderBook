#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>

#include "threadPool.h"
#include "queue.h"
#include "instrument.h"
#include "orderBook.h"

class OrderBookManager {
public:
	explicit OrderBookManager(ThreadPool& threadPool)
		: threadPool_{threadPool}
	{
		dispatchThread_ = std::thread{&OrderBookManager::dispatchThread_, this};
	};

	~OrderBookManager()
	{
		isDone_ = true;
		if (dispatchThread_.joinable()) dispatchThread_.join();
	};

	void AddOrder(InstrumentSymbol& symbol, OrderPointer order)
	{
		auto execUnit = GetOrCreateInstrumentExecutionUnit(symbol);
		execUnit->taskQueue_.push([=]() -> void
			{
				execUnit->orderBook_->AddOrder(order);
			}
		);
	}

	void ModifyOrder(InstrumentSymbol& symbol, OrderId orderId, Side side, Price price, Quantity quantity)
	{
		auto execUnit = GetOrCreateInstrumentExecutionUnit(symbol);
		execUnit->taskQueue_.push([=]() -> void
			{
				execUnit->orderBook_->ModifyOrder(orderId, side, price, quantity);
			}
		);
	}

	void CancelOrder(InstrumentSymbol& symbol, OrderId orderId)
	{
		auto execUnit = GetOrCreateInstrumentExecutionUnit(symbol);
		execUnit->taskQueue_.push([=]() -> void
			{
				execUnit->orderBook_->CancelOrder(orderId);
			}
		);
	}

private:
	struct InstrumentExecutionUnit
	{
		std::unique_ptr<OrderBook> orderBook_;
		ThreadSafeQueue<std::function<void()>> taskQueue_;
		std::atomic_bool isRunning_;

		explicit InstrumentExecutionUnit()
		: isRunning_{false} {};
	};

	mutable std::mutex managerMut_;
	ThreadPool& threadPool_;
	std::atomic_bool isDone_;
	std::thread dispatchThread_;
	std::unordered_map<InstrumentSymbol, std::shared_ptr<InstrumentExecutionUnit>> execUnits_;

	std::shared_ptr<InstrumentExecutionUnit> GetOrCreateInstrumentExecutionUnit(const InstrumentSymbol& symbol);

	void DispatchLoop();

};
