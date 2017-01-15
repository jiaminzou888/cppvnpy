#pragma once

#include "CtaBase.h"
#include "TechIndicator.h"

// 所有策略类型的基类，提取公共接口和方法，所有的交易策略均需要继承它
class CtaEngine;
class StrategyBase
{
public:
	virtual void onInit() = 0;
	virtual void onStart() = 0;
	virtual void onStop() = 0;

	virtual void onTick(QuoteInfo& quote) = 0;
	virtual void onBar(CtaBarData& trade) = 0;
	virtual void onOrder(OrderInfo& order) = 0;
	virtual void onTrade(TradeInfo& trade) = 0;
	
public:
	// 构造函数
	StrategyBase(CtaEngine* ce, std::string name, std::string symbol);

	// 基本功能
	int convert_time_str2int(const char* update_time);

	// 策略相关，委托下单
	std::string buy(double price, int volume, bool stop = false);
	std::string sell(double price, int volume, bool stop = false);
	std::string short_(double price, int volume, bool stop = false);
	std::string cover(double price, int volume, bool stop = false);

	// 撤单
	void cancelOrder(std::string orderID);

private:
	std::string sendOrder(char order_type, double price, int volume, bool stop = false);

public:
	// 落地数据库
	// ..

	// 策略基本参数
	std::string name;		// 策略实例名称
	std::string vtSymbol;	// 交易合约，暂定只交易一种合约

	// 策略的基本变量，由引擎管理
	bool inited{ false };	// 是否进行了初始化
	bool trading{ false };	// 是否启动交易
	int pos{ 0 };			// 持仓情况

	CtaEngine* ce{ nullptr };
};