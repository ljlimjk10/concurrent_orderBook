#pragma once

#include "commonAlias.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <__random/random_device.h>

struct Helper
{
	static OrderId generateRandomOrderId()
	{
		static boost::random::mt19937_64 rng{std::random_device{}()};
		static boost::random::uniform_int_distribution<OrderId> dist;

		return dist(rng);
	}
};
