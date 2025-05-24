#pragma once

#include <map>
#include <unordered_map>
#include "order.h"

#include "commonAlias.h"
#include "trade.h"
#include "priceLevelInfo.h"


class OrderBook
{
public:
	OrderBook() = default;

	// delete copy and move ctors and assignments
	// TODO: to be implemented later
	OrderBook(const OrderBook& other) = delete;
	OrderBook(OrderBook&& other) = delete;

	OrderBook& operator=(const OrderBook&) = delete;
	OrderBook& operator=(OrderBook&&) = delete;

	struct OrderBookLevelInfo
	{
		OrderBookLevelInfo(const PriceLevelInfos& bidsLevelInfos, const PriceLevelInfos& asksLevelInfos)
			: bidsLevelInfos_{bidsLevelInfos}
			, asksLevelInfos_{asksLevelInfos}
		{};

		const PriceLevelInfos& GetBidsLevelData() const { return bidsLevelInfos_ ;};

		const PriceLevelInfos& GetAsksLevelData() const { return asksLevelInfos_ ;};

		PriceLevelInfos bidsLevelInfos_;
		PriceLevelInfos asksLevelInfos_;
	};

	Trades AddOrder(OrderPointer order);

	void CancelOrder(OrderId orderId);

	void ModifyOrder(OrderId orderId, Side side, Price price, Quantity quantity);

	bool CanFullyFillOrder(Side side, Price price, Quantity quantity);

	std::size_t Size() const
	{
		return orders_.size();
	}

	OrderBookLevelInfo GetOrderBookInfo() const;

private:
	struct OrderEntry
	{
		OrderPointer order_;
		OrdersPointers::iterator location_;
	};

	struct PriceLevelData
	{
		Quantity totalOrders_;
		Quantity availableQuantity_;

		enum class Action
		{
			Add,
			Remove,
			Match,
		};
	};

	using PriceLevelDatas = std::vector<PriceLevelData>;


	std::map<Price, PriceLevelData, std::greater<>> priceLevel_;
	std::unordered_map<OrderId, OrderEntry> orders_;
	std::map<Price, OrdersPointers, std::greater<>> bids_;
	std::map<Price, OrdersPointers, std::less<>> asks_;

	void OnCancelOrder(OrderPointer order);

	void OnAddOrder(OrderPointer order);

	void OrderBook::OnMatchOrder(Price price, Quantity quantity, bool isFullyFilled);

	void UpdatePriceLevelData(Price price, Quantity quantity, PriceLevelData::Action action);

	Trades MatchOrder();

};