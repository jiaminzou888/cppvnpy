#pragma once

#include "CtaStrategyBase.h"

class CtaEngine;
class StrategyAtrRsi :public StrategyBase
{
public:
	StrategyAtrRsi(CtaEngine* ce, std::string name, std::string symbol);

	virtual void onInit() override;
	virtual void onStart() override;
	virtual void onStop() override;

	virtual void onTick(QuoteInfo& quote) override;
	virtual void onBar(CtaBarData& bar) override;
	virtual void onOrder(OrderInfo& order) override;
	virtual void onTrade(TradeInfo& trade) override;

private:
	// 策略参数
	int atrLength{ 22 };			// 计算ATR指标的窗口数
	int atrMaLength{ 10 };			// 计算ATR均线的窗口数
	int rsiLength{ 5 };				// 计算RSI的窗口数
	double rsiEntry{ 16 };			// RSI的开仓信号
	double trailingPercent{ 0.8 };	// 百分比移动止损
	int initDays{ 10 };				// 初始化数据所用的天数

	// 策略变量
	CtaBarData bar;					// K线对象
	int barMinute{ 0 };				// K线当前的分钟

	int bufferSize{ 100 };			// 需要缓存的数据的大小
	int bufferCount{ 0 };			// 目前已经缓存了的数据的计数
	std::vector<double> highArray;	// K线最高价的数组
	std::vector<double> lowArray;	// K线最低价的数组
	std::vector<double> closeArray;	// K线收盘价的数组

	int atrCount{ 0 };				// 目前已经缓存了的ATR的计数
	std::vector<double> atrArray;	// ATR指标的数组
	double atrValue{ 0.0 };			// 最新的ATR指标数值
	double atrMa{ 0 };				// ATR移动平均的数值

	double rsiValue{ 0.0 };			// RSI指标的数值
	double rsiBuy{ 0.0 };			// RSI买开阈值
	double rsiSell{ 0.0 };			// RSI卖开阈值
	double intraTradeHigh{ 0.0 };	// RSI移动止损用的持仓期内最高价
	double intraTradeLow{ 0.0 };	// 移动止损用的持仓期内最低价

	std::vector<std::string> orderList;	// 保存委托代码的列表
};