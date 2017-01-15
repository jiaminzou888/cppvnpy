#pragma once

#include "ctpapi/ThostFtdcTraderApi.h"
#pragma comment(lib,"ctpapi/thosttraderapi.lib") 

#include <atomic>

#include "CtpCommand.h"
#include "EventEngine.h"
#include "DataEngine.h"

class CtpTdApi :public QObject, public CThostFtdcTraderSpi 
{
	Q_OBJECT
public:
	CtpTdApi(EventEngine* ee, DataEngine* de) : QueryQueue(1000), TradeQueue(1)
	{
		this->ee = ee;
		this->de = de;
	}

public:
	// 获取连接和登录状态
	bool get_is_td_connect();
	bool get_is_td_logout();

	// API初始化与释放
	void ctp_td_init(QString tdAddress, QString userid, QString password, QString brokerid);
	void ctp_td_release();

	// 查询刷新页面
	void ctp_td_query(Event ev);

	// 用户登录登出
	void ctp_td_login();
	void ctp_td_logout();

	// 交易API基本信息查询
	void ctp_td_getSettlement();
	void ctp_td_getInstrument();
	void ctp_td_getCommission(QString ins_id);
	void ctp_td_getAccount();
	void ctp_td_getPosition();

	// 委托单买卖撤单查
	QString ctp_td_send_limitOrder(TThostFtdcInstrumentIDType instrumentid, TThostFtdcPriceType price, TThostFtdcVolumeType volume, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset);
	QString ctp_td_send_marketOrder(TThostFtdcInstrumentIDType instrumentid, TThostFtdcVolumeType volume, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset);
	void ctp_td_cancelOrder(TThostFtdcInstrumentIDType instrumentID, TThostFtdcExchangeIDType exchangeID, TThostFtdcOrderRefType orderID, TThostFtdcFrontIDType frontID, TThostFtdcSessionIDType sessionID);

private:
	// 判断是否错误消息
	bool is_error_rsp(CThostFtdcRspInfoField *pRspInfo);
	// 委托单公共字段填写并下单
	QString ctp_td_order_insert(CThostFtdcInputOrderField& order_fields);
	// 委托单状态更新
	void ctp_td_order_update(OrderInfo& order);

private:
	// 事件引擎与查询/交易队列
	EventEngine* ee;
	DataEngine* de;

	CommandQueue QueryQueue;
	CommandQueue TradeQueue;

	//交易API
	CThostFtdcTraderApi* TdApi;

	// 用户交易信息
	user_trade_info usr_td_info;
	
	//是否连接/可交易/登出
	std::atomic <bool>	is_td_connect{ false };
	std::atomic <bool>  is_td_tradable{ false };
	std::atomic<bool>	is_td_logout{ false };

	// 查询事件计数
	int query_count{ 0 };
	int query_trgger{ 3 };
	int query_function_index{ 0 };

private:
	/****************************Spi回调函数****************************************/
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected() override;
	///当客户端与交易后台通信连接断开时，该方法被调用。
	void OnFrontDisconnected(int nReason);
	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///登出请求响应
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	//投资者结算结果确认响应
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///报单录入请求响应(参数不通过)
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///撤单操作请求响应(参数不通过)
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///报单通知
	void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
	///成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade) override;
	///请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///请求查询资金账户响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///请求查询合约手续费率响应
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///请求查询合约响应
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
};