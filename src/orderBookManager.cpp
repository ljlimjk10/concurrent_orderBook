#include "../include/orderBookManager.h"

std::shared_ptr<OrderBookManager::InstrumentExecutionUnit> OrderBookManager::GetOrCreateInstrumentExecutionUnit(const InstrumentSymbol& symbol)
{
	std::lock_guard<std::mutex> lock(managerMut_);
	if (execUnits_.contains(symbol)) return execUnits_[symbol];
	auto execUnit = std::make_shared<InstrumentExecutionUnit>();
	execUnit->orderBook_ = std::make_unique<OrderBook>();
	execUnits_[symbol] = execUnit;
	return execUnit;
}

void OrderBookManager::DispatchLoop()
{
	while (!isDone_)
	{
		std::lock_guard<std::mutex> lock{managerMut_};
		for (auto& [symbol, execUnit] : execUnits_)
		{
			bool expected = false;
			if (execUnit->isRunning_.compare_exchange_strong(expected, true))
			{
				std::function<void()> task;
				if (execUnit->taskQueue_.tryPop(task))
				{
					threadPool_.submit([task, execUnit]()
					{
						task();
						execUnit-> isRunning_.store(false);
					});
				}
				else execUnit-> isRunning_.store(false);
			}
		}
		std::this_thread::yield();
	}
}
