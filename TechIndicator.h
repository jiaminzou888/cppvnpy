#pragma once

#include <vector>

class TechIndicator
{
public:
	using priceIn = const std::vector<double>&;
	using indicOut = std::vector<double>&;

	static bool initialize();	// 必须调用一次，且仅调用一次
	static bool taShutdown();	// 必须调用一次，且仅调用一次

	static bool ATR(priceIn high, priceIn low, priceIn close, int window, int& size, indicOut out);
	static bool MA(priceIn price, int window, int& size, indicOut out);
	static bool RSI(priceIn price, int window, int& size, indicOut out);
};