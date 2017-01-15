#include "TechIndicator.h"

// talib头文件
#include "talib/include/ta_libc.h"

// _cmd : Multithreaded + Debug info
#pragma comment(lib,"talib/lib/ta_libc_cdr.lib")

bool TechIndicator::initialize()
{
	int ret = TA_Initialize();
	if (TA_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TechIndicator::taShutdown()
{
	int ret = TA_Shutdown();
	if (TA_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TechIndicator::ATR(priceIn high, priceIn low, priceIn close, int window, int& size, indicOut out)
{
	const int beg_idx = 0;
	const int end_idx = high.size();

	int out_beg = -1;
	out.resize(end_idx); // 需要预分配内存

	// 计算结果测试正确
	int ret = TA_ATR(beg_idx, end_idx - 1, &high[0], &low[0], &close[0], window, &out_beg, &size, &out[0]);
	if (TA_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TechIndicator::MA(priceIn price, int window, int& size, indicOut out)
{
	const int beg_idx = 0;
	const int end_idx = price.size();

	int out_beg = -1;
	out.resize(end_idx); // 需要预分配内存

	// 计算结果测试正确
	int ret = TA_MA(beg_idx, end_idx - 1, &price[0], window, TA_MAType_SMA, &out_beg, &size, &out[0]);
	if (TA_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TechIndicator::RSI(priceIn price, int window, int& size, indicOut out)
{
	const int beg_idx = 0;
	const int end_idx = price.size();

	int out_beg = -1;
	out.resize(end_idx);  // 需要预分配内存

	// 计算结果测试正确
	int ret = TA_RSI(beg_idx, end_idx - 1, &price[0], window, &out_beg, &size, &out[0]);
	if (TA_SUCCESS == ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}