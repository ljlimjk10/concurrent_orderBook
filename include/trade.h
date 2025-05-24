#pragma once

#include <memory>
#include "TradeInfo.h"

class Trade
{
public:
	Trade(std::shared_ptr<TradeInfo> bidInfo, std::shared_ptr<TradeInfo> askInfo)
		: bidInfo_{std::move(bidInfo)}
		, askInfo_{std::move{askInfo}}
	{}
private:
	std::shared_ptr<TradeInfo> bidInfo_;
	std::shared_ptr<TradeInfo> askInfo_;
};

using Trades = std::vector<Trade>;