#pragma once

#include <string>
#include <memory>

#include "ctpapi/ThostFtdcUserApiDataType.h"
#include "Model//PublicStruct.h"

// 本地停止单前缀
const std::string STOPORDERPREFIX = "CtaStopOrder.";

enum OrderType
{
	CTAORDER_BUY = '0',		// 买开
	CTAORDER_SELL = '1',	// 卖平
	CTAORDER_SHORT = '2',	// 卖开
	CTAORDER_COVER = '3',	// 买平
};

// 本地停止单状态
enum StopStatus
{
	STOPORDER_WAITING = '0',	// 等待中
	STOPORDER_CANCELLED = '1',	// 已撤销
	STOPORDER_TRIGGERED = '2'	// 已触发
};

class StrategyBase;
struct StopOrder
{
	std::string vtSymbol;
	char orderType{ 0 };
	char direction{ 0 };
	char offset{ 0 };
	double price{ 0 };
	int volume{ 0 };

	// 下停止单的策略对象
	std::shared_ptr<StrategyBase>	strategy;
	// 停止单的本地编号 
	std::string stopOrderID;
	// 停止单状态
	std::string status;
};

struct CtaBarData
{
	std::string vtSymbol{ "" };
	std::string symbol{ "" };
	std::string exchange{ "" };
	
	double open{ 0 };
	double high{ 0 };
	double low{ 0 };
	double close{ 0 };

	int volume{ 0 };
	int openInterest{ 0 };

	int date{ 0 };
	int time{ 0 };

	bool is_new{ true };
};

// pos已由上层CPositionBuffer在持仓回调中做过处理,因此无需考虑区分交易所。（需要debug查看具体实现）
class CtaPositionBuffer
{
public:
	// 更新持仓数据
	inline void updatePositionData(PositionInfo pos)
	{
		if (THOST_FTDC_PD_Long == pos.direction)
		{
			longPosition = pos.position;
			longYd = pos.ydPosition;
			longToday = longPosition - longYd;
		}
		else
		{
			shortPosition = pos.position;
			shortYd = pos.ydPosition;
			shortToday = shortPosition - shortYd;
		}
	}
	// 更新成交数据
	inline void updateTradeData(TradeInfo trade)
	{
		// 买入
		if (THOST_FTDC_D_Buy == trade.direction)
		{
			// 多方开仓，则对应多头的持仓和今仓增加
			if (THOST_FTDC_OF_Open == trade.offset)
			{
				longPosition += trade.volume;
				longToday += trade.volume;
			}
			// 多方平今，对应空头的持仓和今仓减少
			else if (THOST_FTDC_OF_CloseToday == trade.offset)
			{
				shortPosition -= trade.volume;
				shortToday -= trade.volume;
			}
			// 多方平昨，对应空头的持仓和昨仓减少
			else
			{
				shortPosition -= trade.volume;
				shortYd -= trade.volume;
			}
		}
		// 卖出
		else
		{
			// 空方开仓，则对应空方的持仓和今仓增加
			if (THOST_FTDC_OF_Open == trade.offset)
			{
				shortPosition += trade.volume;
				shortToday += trade.volume;
			}
			// 空方平今，对应多头的持仓和今仓减少
			else if (THOST_FTDC_OF_CloseToday == trade.offset)
			{
				longPosition -= trade.volume;
				longToday -= trade.volume;
			}
			// 空方平昨，对应多头的持仓和昨仓减少
			else
			{
				longPosition -= trade.volume;
				longYd -= trade.volume;
			}
		}
	}

public:
	std::string vtSymbol;
	// 多头
	int longPosition{ 0 };
	int longToday{ 0 };
	int longYd{ 0 };
	// 空头
	int shortPosition{ 0 };
	int shortToday{ 0 };
	int shortYd{ 0 };
};