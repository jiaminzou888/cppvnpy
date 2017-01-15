#pragma once


#include <QMap>
#include <QStringList> 

#include "Model/PublicStruct.h"
#include "Model/PositionBuffer.h"

struct user_trade_info
{
	// 登录帐号信息
	TThostFtdcBrokerIDType	brokerid;
	TThostFtdcUserIDType	userid;
	TThostFtdcPasswordType	password;
	// 交易日
	TThostFtdcDateType trading_day;

	//请求编号
	int requestID{ 0 };
	//本地的最大报单引用
	int maxOrderRef{ 0 };

	// 会话标记
	int front_id{ 0 };
	int session_id{ 0 };

	user_trade_info()
	{
		memset(trading_day, 0x00, sizeof(trading_day));
	}
};

struct orderCommonRequest
{
	char instrument[31];
	double price;
	int volume;
	char direction;
	char offset;

	orderCommonRequest()
	{
		memset(this, 0x00, sizeof(orderCommonRequest));
	}
};

struct cancelCommonRequest
{
	char instrument[31];
	char exchange[9];

	// CTP, LTS相关
	char order_ref[13];
	int session_id;
	int front_id;

	cancelCommonRequest()
	{
		memset(this, 0x00, sizeof(cancelCommonRequest));
	}
};

// 存储模块公共数据
class DataEngine: public QObject
{
public:
	bool de_get_contract(QString vtSymbol, InstrumentInfo& contract);
	bool de_get_order(QString ordID, OrderInfo& ordInfo);
	

public:
	/***MD Interface***/
	// Tick回调（tick需要提炼一下）
	QMap <QString, CThostFtdcDepthMarketDataField> lastMarketDataSet;

	/***TD Interface***/
	// 账户信息
	AccountInfo accountInfo;
	// 持仓查询缓存（区分昨仓和今仓）<key: vtSymbol.持仓方向>，本地所维护的真正持仓数据
	QMap<QString, CPositionBuffer>	allPosition_buffer;
	// 持仓查询 <key: vtSymbol.持仓方向>，仅用于用户持仓列表展示的持仓备份
	QMap<QString, PositionInfo>		allPosition;
	// 存放所有合约信息，供UI界面展示
	QMap <QString, InstrumentInfo>	allInstruments;
	// 交易开启后的所有委托
	QMap<QString, OrderInfo>		allOrderDict;
	// 交易活动委托
	QMap<QString, OrderInfo>		workingOrderDict;
};