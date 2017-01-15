#pragma once

#include "PublicStruct.h"
#include "..\ctpapi\ThostFtdcUserApiStruct.h"

class CPositionBuffer
{
public:
	void setPositionBuffer(CThostFtdcInvestorPositionField* data, QString gatewayName)
	{
		symbol = data->InstrumentID;
		direction = data->PosiDirection;

		pos_.symbol = (symbol);
		pos_.vtSymbol = (symbol);
		pos_.gatewayName = (gatewayName);
		pos_.direction = (direction);
		pos_.vtPositionName = (pos_.vtPositionName + QString(".") + QString(direction));
	}

	const PositionInfo& get_position()
	{
		return pos_;
	}

	// 更新上期所缓存，返回更新后的持仓数据
	PositionInfo updateShfeBuffer(CThostFtdcInvestorPositionField* data, int size)
	{
		// 昨仓和今仓的数据更新是分在两条记录里的，因此需要判断检查该条记录对应仓位
		// 因为今仓字段TodayPosition可能变为0（被全部平仓），因此分辨今昨仓需要用YdPosition字段
		if (data->YdPosition)
		{
			yd_postion = data->Position;
			yd_position_cost = data->PositionCost;
		}
		else
		{
			today_position = data->Position;
			today_position_cost = data->PositionCost;
		}

		// 持仓的昨仓和今仓相加后为总持仓
		pos_.position = (today_position + yd_postion);
		pos_.ydPosition = (yd_postion);

		// 如果手头还有持仓，则通过加权平均方式计算持仓均价
		if (today_position || yd_postion)
		{
			pos_.price = ((yd_position_cost + today_position_cost) / ((yd_postion + today_position) * size));
		}
		// 否则价格为0
		else
		{
			pos_.price = (0);
		}

		// 多空改名
		if (pos_.direction == '2')
		{
			pos_.directName = ("Long");
		}
		else
		{
			pos_.directName = ("Short");
		}

		return pos_;
	}

	// 更新其他交易所的缓存，返回更新后的持仓数据
	PositionInfo updateBuffer(CThostFtdcInvestorPositionField* data, int size)
	{
		// 其他交易所并不区分今昨，因此只关心总仓位，昨仓为0
		pos_.position = (data->Position);
		pos_.ydPosition = (0);

		if (data->Position)
		{
			pos_.price = (data->PositionCost / (data->Position * size));
		}
		else
		{
			pos_.price = (0);
		}

		// 多空改名
		if (pos_.direction == '2')
		{
			pos_.directName = ("Long");
		}
		else
		{
			pos_.directName = ("Short");
		}

		return pos_;
	}

	// 缓存的yd_postion 和 pos_.yd_position都是当前昨仓。回调中的YdPosition仅用于上期所昨仓和今仓判断
private:
	// 记入当前一次回调所传进来的数据：区分 昨仓 和 今仓
	QString symbol;
	QChar direction{ 0 };
	int today_position{ 0 };
	int yd_postion{ 0 };
	double today_position_cost{ 0 };
	double yd_position_cost{ 0 };

	// 汇总当前持仓数据的持仓，即历史持仓缓存，待新一次回调进入时更新。
	// 这才是本地维护的真正持仓数据
	PositionInfo pos_;
};