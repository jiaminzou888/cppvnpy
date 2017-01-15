#pragma once

#include <map>
#include <list>
#include <memory>

#include "EventBase.h"
#include "CtaBase.h"

// 前置声明，防止循环引用 
class MainEngine;
class EventEngine;

class CtaEngine
{
public:
	CtaEngine(MainEngine* me, EventEngine* ee);
	~CtaEngine();

	// 注册事件回调
	void registerEvent();

	std::string sendOrder(const std::string& vtSymbol, char order_type, double price, int volume, std::shared_ptr<StrategyBase> stg);
	void cancelOrder(const std::string& order_id);
	std::string sendStopOrder(const std::string& vtSymbol, char order_type, double price, int volume, std::shared_ptr<StrategyBase> stg);
	void cancelStopOrder(const std::string& stop_id);

	// 策略加载与初始化
	void processStartStrategy(Event ev);
	void processStopStrategy(Event ev);

	// 事件回调处理函数
	void procecssStopOrderEvent(const QuoteInfo& quote);
	void procecssTickEvent(Event ev);
	void processOrderEvent(Event ev);
	void processTradeEvent(Event ev);
	void processPositionEvent(Event ev);

private:
	void loadStrategy();
	void initStrategy(const std::string& stg_name);
	void startStrategy(const std::string& stg_name);
	void stopStrategy(const std::string& stg_name);

private:
	// 当前日期
	int todayDate{ 0 };
	int stopOrderCount{ 0 };

	// 策略实例应当初始化
	// key为策略名称，value为策略实例
	std::map<std::string, std::shared_ptr<StrategyBase>> strategyDict;
	// 由于可能多个strategy交易同一个vtSymbol，因此key为vtSymbol,value为包含所有相关strategy对象的list,
	// 它在作用仅仅在于观察是否有策略对某合约感兴趣，必须首先满足感兴趣，才做其他处理。
	std::map<std::string, std::list<std::shared_ptr<StrategyBase>>> tickStrategyDict;
	// key为OrderID，value为策略实例(策略到CTP的发单)
	std::map<std::string, std::shared_ptr<StrategyBase>> orderStrategyDict;

	// key为stopOrderID，value为stopOrder对象(策略到的停止单)
	std::map<std::string, StopOrder> stopOrderDict;
	std::map<std::string, StopOrder> workingStopOrderDict;

	// key为vtSymbol，value为PositionBuffer对象
	std::map<std::string, CtaPositionBuffer> posBufferDict;
private:
	MainEngine* me{ nullptr };
	EventEngine* ee{ nullptr };
};
