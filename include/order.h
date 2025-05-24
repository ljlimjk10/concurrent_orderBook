#pragma once

#include <list>
#include <fmt/core.h>
#include <stdexcept>

#include "commonAlias.h"
#include "instrument.h"

enum class Side
{
	Buy,
	Sell,
};

enum class OrderType
{
	MarketOrder,
	LimitOrder,
	FillOrKillOrder,
	DayOrder,
	GoodTillCancelOrder,
};

// Order can be in multiple areas
class Order;
using OrderPointer = std::shared_ptr<Order>;
using OrdersPointers = std::list<OrderPointer>;

class Order
{
public:
	// for market order
	Order(OrderId orderId, OrderType orderType, Side side, Quantity initialQuantity)
		: orderId_{orderId}
		, orderType_{orderType}
		, side_{side}
		, initialQuantity_{initialQuantity}
		, remainingQuantity_{initialQuantity}
		, price_{INVALID_PRICE}
		, timestamp_{std::chrono::system_clock::now()}
	{};

	// other orders
	Order(OrderId orderId, OrderType orderType, Side side, Quantity initialQuantity, Price price)
		: orderId_{orderId}
		, orderType_{orderType}
		, side_{side}
		, initialQuantity_{initialQuantity}
		, remainingQuantity_{initialQuantity}
		, price_{price}
		, timestamp_{std::chrono::system_clock::now()}
	{};

	Order() = default;

	constexpr Price INVALID_PRICE = -1.0;

	OrderId getOrderId() const {return orderId_;}

	OrderType getOrderType() const {return orderType_;};

	Side getSide() const {return side_;};

	Quantity getInitialQuantity() const {return initialQuantity_;};

	Quantity getRemainingQuantity() const {return remainingQuantity_;};

	Price getPrice() const {return price_;};

	bool isFilled() const {return remainingQuantity_ == 0;};

	void adjustMarketOrderPrice(Price price)
	{
		if (getOrderType() != OrderType::MarketOrder)
		{
			throw std::runtime_error(fmt::format("orderId {}: price of non market order cannot be adjusted", getOrderId()));
		}
		price_ = price;
	}

	void fillOrder(Quantity quantity)
	{
		if (quantity > getRemainingQuantity())
		{
			throw std::runtime_error("quantity cannot be greater than remaining quantity");
		}
		setRemainingQuantity(getRemainingQuantity()-quantity);
	}

private:
	OrderId orderId_;
	OrderType orderType_;
	Side side_;
	Quantity initialQuantity_;
	Quantity remainingQuantity_;
	Price price_;
	Timestamp timestamp_;

	void setRemainingQuantity(Quantity quantity)
	{
		remainingQuantity_ = quantity;
	}
};

