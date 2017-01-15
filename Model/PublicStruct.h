#pragma once

#include <QString>
#include <QDateTime>

//记录账户的基本信息：用户ID，交易账号，账号密码，经纪商代码，前置机地址
struct AccountID
{
public:
	AccountID& operator=(const AccountID&) = delete;

public:
	QString apiName;
	QString investorID;
	QString password;
	QString brokerID;
	QString frontAddress;
	int connectionStatus;
};

// 用户帐号资金账户信息
struct AccountInfo
{
	// vnpy帐号相关
	QString id;
	QString vtId;
	QString gatewayName;

	// vnpy数值相关
	double preBalance;
	double balance;
	double available;
	double commission;
	double margin;
	double close_profit;
	double position_profit;
};

//合约信息：id、名字、交易所编号、最后交割日、保证金率、合约乘数、保证金率、手续费、最小变动单位、现在是否可以交易
struct InstrumentInfo
{
public:
	//为了放在set集合中，重载 < 运算符
	bool operator < (const InstrumentInfo &instr) { return id < instr.id; };

public:
	QString id;
	QString name;
	QString exchangeId;
	QDate deadline;

	double marginRate;
	int multiplier;
	double openCommission;
	double closeCommission;
	double closeTodayCommission;
	double minimumUnit;
	bool tradable;

	bool has_subsribed{ false };
};

// 委托单信息
struct OrderInfo
{
	// 代码编号相关
	QString symbol;
	QString vtSymbol;
	QString exchange;

	QString orderID;
	QString vtOrderID;

	// 报单相关
	QChar direction;
	QChar offset;
	double price;
	int totalVolume;
	int tradeVolume;
	QChar status;

	QString orderTime;
	QString cancelTime;

	int frontID;
	int sessionID;

	QString gatewayName;
};

// 成交信息
struct TradeInfo
{
	// 代码编号相关
	QString symbol;
	QString vtSymbol;
	QString exchange;

	QString tradeID;
	QString vtTradeID;

	QString orderID;
	QString vtOrderID;

	// 成交相关
	QChar direction;
	QChar offset;
	double price;
	int volume;
	QString tradeTime;

	QString gatewayName;
};

// 持仓信息
struct PositionInfo
{
	// 考虑到现阶段大部分CTP中的ExchangeID字段返回的都是空值, vtSymbol直接使用symbol
	QString symbol;
	QString vtSymbol;

	QString directName;
	QChar direction{ 0 };
	int position{ 0 };

	int frozen{ 0 };
	double price{ 0 };

	QString vtPositionName;
	int ydPosition{ 0 };

	QString gatewayName;
};

// Tick报价
struct QuoteInfo
{
	QString symbol;
	QString vtSymbol;

	int volume;
	int openInterest;

	double openPrice;
	double highPrice;
	double lowPrice;
	double lastPrice;

	double bidPrice1;
	int bidVolume1;
	double askPrice1;
	int askVolume1;

	double upperLimit;
	double lowerLimit;

	double preClosePrice;

	QString time;
	QString gatewayName;
};