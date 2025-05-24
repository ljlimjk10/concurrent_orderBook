#pragma once

#include <string>

using InstrumentSymbol = std::string;

class Instrument
{
public:
	Instrument(std::string symbol, bool isActive)
		: symbol_{symbol}
		, isActive_{isActive}
	{};

	const std::string& getSymbol() const {return symbol_;};

	bool getIsActive() const {return isActive_;};

private:
	InstrumentSymbol symbol_;
	bool isActive_;
};

