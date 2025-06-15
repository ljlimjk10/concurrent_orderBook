#include "../include/orderBook.h"

#include <numeric>

#include "../include/trade.h"
#include "../include/helper.h"

Trades OrderBook::AddOrder(OrderPointer order)
{
	// check if this order id exist alr
	if (orders_.contains(order->getOrderId())) return {};

	// adjust price for market order to take the best opposing price
	if (order->getOrderType() == OrderType::MarketOrder)
	{
		if (order->getSide() == Side::Buy && !asks_.empty())
		{
			// use auto& to avoid copying shared_ptr and ++ its ref count
			const auto& [bestPrice, _] = *asks_.begin();
			order->adjustMarketOrderPrice(bestPrice);
		}
		else if (order->getSide() == Side::Sell && !bids_.empty())
		{
			const auto& [bestPrice, _] = *bids_.begin();
			order->adjustMarketOrderPrice(bestPrice);
		}
		else return {};
	}

	if (order->getOrderType() == OrderType::FillOrKillOrder)
	{
		if (!CanFullyFillOrder(order->getSide(), order->getPrice(), order->getInitialQuantity())) return {};
	}

	OrdersPointers::iterator iterator;
	if (order->getSide() == Side::Buy)
	{
		auto& orders = bids_[order->getPrice()];
		orders.push_back(order);
		iterator = std::prev(orders.end());
	}
	else
	{
		auto& orders = asks_[order->getPrice()];
		orders.push_back(order);
		iterator = std::prev(orders.end());
	}
	orders_.emplace(order->getOrderId(), OrderEntry{order, iterator});

	// add to priceLevels
	OnAddOrder(order);

	// match orders
	return MatchOrder();
}

void OrderBook::CancelOrder(OrderId orderId)
{
	// remove order from orders_ and from bids_ or asks_ and priceLevel
	if (!orders_.contains(orderId)) return;

	const auto& [order, orderIterator] = orders_[orderId];
	orders_.erase(orderId);

	if (order->getSide() == Side::Buy)
	{
		Price price = order->getPrice();
		auto& orders = bids_.at(price);
		orders.erase(orderIterator);
		if (orders.empty()) bids_.erase(price);
	}
	else
	{
		Price price = order->getPrice();
		auto& orders = asks_.at(price);
		orders.erase(orderIterator);
		if (orders.empty()) asks_.erase(price);
	}
	OnCancelOrder(order);
}

void OrderBook::ModifyOrder(OrderId orderId, Side side, Price price, Quantity quantity)
{
	const auto& [existingOrder, orderIterator] = orders_[orderId];
	OrderType existingOrderType = existingOrder->getOrderType();
	Price existingPrice = existingOrder->getPrice();
	Side existingSide = existingOrder->getSide();

	CancelOrder(orderId);

	OrderId newOrderId = Helper::generateRandomOrderId();
	Order newOrder = (existingOrderType == OrderType::MarketOrder)
	? Order{newOrderId, existingOrderType, side, quantity}
	: Order{newOrderId, existingOrderType, side, quantity, price};

	AddOrder(std::make_shared<Order>(newOrder));
}

bool OrderBook::CanFullyFillOrder(Side side, Price limitPrice, Quantity quantity)
{
	if (side == Side::Buy)
	{
		if (asks_.empty()) return false;

		// any price below currBestPrice, take
		for (auto iter = priceLevel_.rbegin(); iter != priceLevel_.rend(); ++iter)
		{
			const Price price = iter->first;
			const auto& level = iter->second;
			if (price > limitPrice) break;
			quantity -= std::min(quantity, level.availableQuantity_);
			if (quantity == 0) return true;
		}
	}
	else
	{
		if (bids_.empty()) return false;
		// any price above currBestPrice, take
		for (auto iter=priceLevel_.begin(); iter!=priceLevel_.end(); ++iter)
		{
			const Price price = iter->first;
			const auto& level = iter->second;
			if (price < limitPrice) break;
			quantity -= std::min(quantity, level.availableQuantity_);
			if (quantity == 0) return true;
		}
	}
	return false;
}

void OrderBook::UpdatePriceLevelData(Price price, Quantity quantity, PriceLevelData::Action action)
{
	PriceLevelData& currPriceLevelData = priceLevel_[price];
	currPriceLevelData.totalOrders_ += action == PriceLevelData::Action::Add ? 1 : action == PriceLevelData::Action::Remove ? -1 : 0;

	if (action == PriceLevelData::Action::Add)
	{
		currPriceLevelData.availableQuantity_ += quantity;
	}
	else if (action == PriceLevelData::Action::Remove || action == PriceLevelData::Action::Match)
	{
		currPriceLevelData.availableQuantity_ -= quantity;
	}

	if (currPriceLevelData.totalOrders_ == 0)
	{
		priceLevel_.erase(price);
	}
}

void OrderBook::OnAddOrder(OrderPointer order)
{
	UpdatePriceLevelData(order->getPrice(), order->getInitialQuantity(), PriceLevelData::Action::Add);
}

void OrderBook::OnCancelOrder(OrderPointer order)
{
	UpdatePriceLevelData(order->getPrice(), order->getInitialQuantity(), PriceLevelData::Action::Remove);
}

// match might be partial
void OrderBook::OnMatchOrder(Price price, Quantity quantity, bool isFullyFilled)
{
	UpdatePriceLevelData(price, quantity, isFullyFilled ? PriceLevelData::Action::Remove : PriceLevelData::Action::Match);
}

Trades OrderBook::MatchOrder()
{
	Trades tradesMade;
	tradesMade.reserve(orders_.size());

	while (true)
	{
		if (bids_.empty() || asks_.empty()) break;

		auto& [bidPrice, bidOrders] = *bids_.begin();
		auto& [askPrice, askOrders] = *asks_.begin();

		if (bidPrice < askPrice) break;

		while (!bidOrders.empty() && !askOrders.empty())
		{
			std::shared_ptr<Order> bidOrder = bidOrders.front();
			std::shared_ptr<Order> askOrder = askOrders.front();

			Quantity minQuantity = std::min(bidOrder->getRemainingQuantity(), askOrder->getRemainingQuantity());
			bidOrder->fillOrder(minQuantity);
			askOrder->fillOrder(minQuantity);

			if (bidOrder->isFilled())
			{
				bidOrders.pop_front();
				orders_.erase(bidOrder->getOrderId());
			}
			if (askOrder->isFilled())
			{
				askOrders.pop_front();
				orders_.erase(askOrder->getOrderId());
			}

			tradesMade.emplace_back(Trade{std::make_shared<TradeInfo>(TradeInfo{bidOrder->getOrderId(), minQuantity, bidOrder->getPrice()}), std::make_shared<TradeInfo>(TradeInfo{askOrder->getOrderId(), minQuantity, askOrder->getPrice()})});
			OnMatchOrder(bidOrder->getPrice(), minQuantity, bidOrder->isFilled());
			OnMatchOrder(askOrder->getPrice(), minQuantity, askOrder->isFilled());
		}

		if (bidOrders.empty())
		{
			bids_.erase(bidPrice);
		}
		if (askOrders.empty())
		{
			asks_.erase(bidPrice);
		}
	}

	// deal with partially or unfilled FillOrKillOrder
	if (!bids_.empty())
	{
		auto& [_, bidOrders] = *bids_.begin();
		auto& order = bidOrders.front();
		if (order->getOrderType() == OrderType::FillOrKillOrder) CancelOrder(order->getOrderId());
	}

	if (!asks_.empty())
	{
		auto& [_, askOrders] = *asks_.begin();
		auto& order = askOrders.front();
		if (order->getOrderType() == OrderType::FillOrKillOrder) CancelOrder(order->getOrderId());
	}
	return tradesMade;

}

OrderBook::OrderBookLevelInfo OrderBook::GetOrderBookInfo() const
{
	PriceLevelInfos bidsPriceInfos, asksPriceInfos;
	bidsPriceInfos.reserve(orders_.size());
	asksPriceInfos.reserve(orders_.size());

	auto CreatePriceLevelInfo = [](Price price, const OrdersPointers& orders) ->PriceLevelInfo
	{
		return PriceLevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity)0, [](Quantity currSum, const OrderPointer& order) {return currSum+order->getRemainingQuantity();})};
	};

	for (const auto& [price, orders] : bids_)
	{
		bidsPriceInfos.push_back(CreatePriceLevelInfo(price, orders));
	}
	for (const auto& [price, orders] : asks_)
	{
		asksPriceInfos.push_back(CreatePriceLevelInfo(price, orders));
	}

	return OrderBookLevelInfo(bidsPriceInfos, asksPriceInfos);
}


