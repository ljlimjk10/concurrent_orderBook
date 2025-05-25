#include "include/threadPool.h"
#include "include/orderBookManager.h"

int main()
{
	ThreadPool threadPool;
	OrderBookManager orderBookManager{threadPool};

	return 0;
}
