#pragma once

#include "ctpapi/ThostFtdcMdApi.h"
#pragma comment(lib,"ctpapi/thostmduserapi.lib") 

#include <atomic>

#include "EventEngine.h"
#include "DataEngine.h"

class CtpMdApi :public QObject, public CThostFtdcMdSpi
{
	Q_OBJECT

public:
	CtpMdApi(EventEngine *ee, DataEngine* de)
	{
		this->ee = ee;
		this->de = de;
	}

	bool get_is_md_connect();
	bool get_is_md_logout();

	void ctp_md_init(QString mdaddress);
	void ctp_md_release();

	void ctp_md_login();
	void ctp_md_logout();

	void ctp_md_subscribe(QString instrumendid);

private:
	CThostFtdcMdApi* MdApi;
	EventEngine* ee;
	DataEngine* de;

	int requestID{ 0 };
	std::atomic<bool>	is_md_connect{ false };
	std::atomic<bool>	is_md_logout{ false };

private:
	/****************************Spi回调函数****************************************/
	//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用
	void OnFrontConnected() override;
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	void OnFrontDisconnected(int nReason) override;
	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	///登出请求响应
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	//行情回报响应
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;
};