#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>

#include "instrument.h"
#include "orderBook.h"

class OrderBookManager {
public:
	OrderBookManager() = default;

	OrderBook& GetOrderBook(const InstrumentSymbol& symbol)
	{
		std::lock_guard<std::mutex> lock(mut_);
		if (orderBooks_.contains(symbol)) return *orderBooks_[symbol];
		orderBooks_[symbol] = std::make_unique<OrderBook>();
		return *orderBooks_[symbol];
	}

	Trades AddOrder(InstrumentSymbol& symbol, OrderPointer order)
	{
		return GetOrderBook(symbol).AddOrder(order);
	}

	void ModifyOrder(InstrumentSymbol& symbol, OrderId orderId, Side side, Price price, Quantity quantity)
	{
		return GetOrderBook(symbol).ModifyOrder(orderId, side, price, quantity);
	}

	void CancelOrder(InstrumentSymbol& symbol, OrderId orderId)
	{
		return GetOrderBook(symbol).CancelOrder(orderId);
	}

	OrderBook::OrderBookLevelInfo GetOrderBookInfo(const InstrumentSymbol& symbol)
	{
		return GetOrderBook(symbol).GetOrderBookInfo();
	}

private:
	mutable std::mutex mut_;
	std::unordered_map<InstrumentSymbol, std::unique_ptr<OrderBook>> orderBooks_;
};
