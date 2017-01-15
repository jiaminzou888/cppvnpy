#pragma once

#include <atomic>

#include "EventEngine.h"
#include "DataEngine.h"
#include "CtaEngine.h"

#include "CtpMdApi.h"
#include "CtpTdApi.h"

// 框架对外主引擎,管理框架对外的所有公共交易和查询，内部实现不同接口的命令转发
class MainEngine
{
public:
	MainEngine()
	{
		ee = new EventEngine;
		de = new DataEngine;
		ce = std::move(std::shared_ptr<CtaEngine>(new CtaEngine(this, ee)));

		// 传de进去是没办法，目前我们的事件回调不支持参数
		ctpmd = new CtpMdApi(ee, de);
		ctptd = new CtpTdApi(ee, de);
	}

	template<typename T>
	void register_event(std::string type, T* pObj, void (T::*pMemberFunc)(Event))
	{
		ee->addEvent(type, pObj, pMemberFunc);
	}

	// 登录
	void me_login(QString userid, QString password, QString brokerid, QString mdAddress, QString tdAddress);
	// 退出
	void me_logout();
	// 订阅行情
	void me_subscribe(QString instrumentid);

	// 用户登录标记
	bool me_get_is_login();

	// 委托单动作
	QString me_sendDefaultOrder(orderCommonRequest& order_field);
	void me_cancelOrder(cancelCommonRequest& cancel_field);

	// CTA测试
	void me_strat_cta();
	void me_stop_cta();

	// CTA下单查询合约和委托单
	bool me_get_contract(QString vtSymbol, InstrumentInfo& contract);
	bool me_get_order(QString ordID, OrderInfo& ordInfo);
	
	// 查询动作：用户交易情况基本信息查询,主要用于UI界面刷新，后期UI改造要去掉
	AccountInfo me_getAccountInfo() const;
	QMap<QString, PositionInfo> me_getPositionInfo() const;
	QMap<QString, InstrumentInfo> me_getInstrumentInfo() const;
	QMap<QString, CThostFtdcDepthMarketDataField> me_getLastMarketData() const;
	QMap<QString, OrderInfo> me_getOrderInfo() const;
	QMap<QString, OrderInfo> me_getWorkingOrderInfo() const;

private:
	std::atomic<bool> me_is_login{ false };

	// 扩展引擎模块
	EventEngine* ee;
	DataEngine* de;
	std::shared_ptr<CtaEngine> ce;

	// Gateway接口
	CtpMdApi* ctpmd;
	CtpTdApi* ctptd;
};