#pragma once
#include "commonAlias.h"

struct PriceLevelInfo
{
	Price price_;
	Quantity quantity_;
};

using PriceLevelInfos = std::vector<PriceLevelInfo>;
