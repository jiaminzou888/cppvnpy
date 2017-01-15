#include "CtpTdApi.h"
#include <QtDebug>
#include <qdir.h>
#include <string>

#include "CtpCommand.h"

void CtpTdApi::OnFrontConnected()
{
	is_td_connect.store(true);
}

void CtpTdApi::OnFrontDisconnected(int nReason)
{
	is_td_connect.store(false);
}

void CtpTdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!is_error_rsp(pRspInfo) && pRspUserLogin)
	{
		// 登录成功
		qDebug() << "TD Succeed With Login";

		usr_td_info.front_id = pRspUserLogin->FrontID;
		usr_td_info.session_id = pRspUserLogin->SessionID;
		usr_td_info.maxOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		strncpy_s(usr_td_info.trading_day, pRspUserLogin->TradingDay, sizeof(usr_td_info.trading_day));

		//确认结算结果
		ctp_td_getSettlement();
	}
	else
	{
		abort();
	}
}

void CtpTdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!is_error_rsp(pRspInfo) && pUserLogout)
	{
		// 退出标记
		is_td_logout.store(true);

		// 登出成功(所有qDebug的地方输出到日志监控模块)
		qDebug() << "TD  Succeed With  Logout";
	}
	else
	{
		abort();
	}
}

void CtpTdApi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!is_error_rsp(pRspInfo) && pSettlementInfoConfirm)
	{
		// 结算单确认以后才可交易
		is_td_tradable.store(true);

		// 结算单确认成功(所有qDebug的地方输出到日志监控模块)
		qDebug() << "TD  Succeed With  SettlementConfirm";

		ctp_td_getInstrument();//获取所有合约信息
	}
	else
	{
	}
}
void CtpTdApi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void CtpTdApi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
}

void CtpTdApi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	//获取最大报单引用
	usr_td_info.maxOrderRef = std::max(usr_td_info.maxOrderRef, (int)pOrder->OrderRef);

	// 委托单更新事件
	Event orderEvent(EVENT_ORDER);
	orderEvent.data_ = std::move(std::shared_ptr<char>((char*)(new OrderInfo)));

	OrderInfo* order = (OrderInfo*)orderEvent.data_.get();
	order->gatewayName = ("CTP");
	order->symbol = (pOrder->InstrumentID);
	order->exchange = (pOrder->ExchangeID);
	order->vtSymbol = (order->symbol);

	order->orderID = (pOrder->OrderRef);
	order->direction = (pOrder->Direction);
	order->offset = (pOrder->CombOffsetFlag[0]);
	order->status = (pOrder->OrderStatus);

	order->price = (pOrder->LimitPrice);
	order->totalVolume = (pOrder->VolumeTotalOriginal);
	order->tradeVolume = (pOrder->VolumeTraded);
	order->orderTime = (pOrder->InsertTime);
	order->cancelTime = (pOrder->CancelTime);
	order->frontID = (pOrder->FrontID);
	order->sessionID = (pOrder->SessionID);

	// CTP的报单号一致性维护需要基于frontID, sessionID, orderID三个字段
	// 但在本接口设计中，已经考虑了CTP的OrderRef的自增性，避免重复
	// 唯一可能出现OrderRef重复的情况是多处登录并在非常接近的时间内（几乎同时发单）
	// 考虑到VtTrader的应用场景，认为以上情况不会构成问题
	order->vtOrderID = (order->gatewayName + QString(".") + order->orderID);

	// 委托单更新
	ctp_td_order_update(*order);

	// 推送订单处理程序，暂未存在，应该在本地维护委托单的订单状态
	ee->putEvent(orderEvent);
}

void CtpTdApi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	// 成交更新事件
	Event tradeEvent(EVENT_TRADE);
	tradeEvent.data_ = std::move(std::shared_ptr<char>((char*)(new TradeInfo)));

	TradeInfo* trade = (TradeInfo*)tradeEvent.data_.get();
	trade->gatewayName = ("CTP");
	trade->symbol = (pTrade->InstrumentID);
	trade->exchange = (pTrade->ExchangeID);
	trade->vtSymbol = (trade->symbol);

	trade->tradeID = (pTrade->TradeID);
	trade->vtTradeID = (trade->gatewayName + QString(".") + trade->tradeID);
	trade->orderID = (pTrade->OrderRef);
	trade->vtOrderID = (trade->gatewayName + QString(".") + trade->tradeID);
	trade->direction = (pTrade->Direction);
	trade->offset = (pTrade->OffsetFlag);
	trade->price = (pTrade->Price);
	trade->volume = (pTrade->Volume);
	trade->tradeTime = (pTrade->TradeTime);

	// 推送成交处理程序
	ee->putEvent(tradeEvent);
}

// 上期所持仓分 4条 记录返回：				多昨，多今，空昨，空今
// 郑交所、大商所和中金所分 2条 记录返回：	多昨今、空昨今

// 该持仓回调甚至还会将已报未成交的委托单报出，指明委托在持仓中的冻结信息

// CPositionBuffer是按持仓方向区分，内部的_pos代表某合约某方向的持仓，
// 因此就需要将上期所持仓记录合并进来。
void CtpTdApi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	// 返回持仓	
	if (!is_error_rsp(pRspInfo) && pInvestorPosition)
	{
		// 获取数据
		QMap<QString, CPositionBuffer>& allPosition_buffer = de->allPosition_buffer;
		QMap <QString, InstrumentInfo>& allInstruments = de->allInstruments;
		QMap<QString, PositionInfo>& allPosition = de->allPosition;

		// 持仓查询
		QString pos_name = QString(pInvestorPosition->InstrumentID) + QString(".") + QString(QChar(pInvestorPosition->PosiDirection));
		if (allPosition_buffer.find(pos_name) == allPosition_buffer.end())
		{
			CPositionBuffer temp_buffer;
			temp_buffer.setPositionBuffer(pInvestorPosition, QString("CTP"));
			allPosition_buffer.insert(pos_name, temp_buffer);	// 新增
		}

		// 更新持仓缓存,推送持仓回调事件
		Event posEvent(EVENT_POSITION);
		posEvent.data_ = std::move(std::shared_ptr<char>((char*)(new PositionInfo)));

		// PositionInfo 已经是汇总后的结果数据
		PositionInfo* pos = (PositionInfo*)posEvent.data_.get();
		QString exchange_name = allInstruments[pInvestorPosition->InstrumentID].exchangeId;
		int size = allInstruments[pInvestorPosition->InstrumentID].multiplier;

		CPositionBuffer& pos_buffer = allPosition_buffer[pos_name];
		if (0 == exchange_name.compare("SHFE"))
		{
			// 只有上期所才存在今仓和昨仓概念
			*pos = pos_buffer.updateShfeBuffer(pInvestorPosition, size);
			allPosition[pos_name] = *pos;
		}
		else
		{
			// 其他交易所无昨仓概念
			*pos = pos_buffer.updateBuffer(pInvestorPosition, size);
			allPosition[pos_name] = *pos;
		}
		
		// 隔夜不关程序，据说这里持仓会算错
		ee->putEvent(posEvent);

		// 该事件仅更新UI
		if (bIsLast)
		{
			ee->putEvent(Event(EVENT_POSITION_UI));
		}
	}
}

void CtpTdApi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//返回账户信息
	if (!is_error_rsp(pRspInfo) && pTradingAccount)
	{
		// 获取数据
		AccountInfo& accountInfo = de->accountInfo;

		// 账户相关
		accountInfo.gatewayName = ("CTP");
		accountInfo.id = (pTradingAccount->AccountID);
		accountInfo.vtId = (accountInfo.gatewayName + QString(".") + accountInfo.id);

		// 数值相关
		accountInfo.preBalance = (pTradingAccount->PreBalance);
		accountInfo.available = (pTradingAccount->Available);
		accountInfo.commission = (pTradingAccount->Commission);
		accountInfo.margin = (pTradingAccount->CurrMargin);
		accountInfo.close_profit = (pTradingAccount->CloseProfit);
		accountInfo.position_profit = (pTradingAccount->PositionProfit);

		// 这里的balance和快期中的账户不确定是否一样，需要测试
		auto data = pTradingAccount;
		double balance = data->PreBalance - data->PreCredit - data->PreMortgage + data->Mortgage -
			data->Withdraw + data->Deposit + data->CashIn - data->Commission + data->CloseProfit + data->PositionProfit;
		accountInfo.balance = (balance);
		
		if (bIsLast)
		{
			ee->putEvent(Event(EVENT_ACCOUNT));
		}
	}
}

void CtpTdApi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (!is_error_rsp(pRspInfo) && pInstrument)
	{
		// 获取数据
		QMap <QString, InstrumentInfo>&	allInstruments = de->allInstruments;

		// 更新数据
		QString id = pInstrument->InstrumentID;
		if (allInstruments.find(id) != allInstruments.end())
		{
			//如果存在
		}
		else
		{
			// 合约信息table
			InstrumentInfo instrumentInfo;
			instrumentInfo.id = id;
			allInstruments.insert(id, instrumentInfo);
		}

		//写入内存对象中
		InstrumentInfo& info = allInstruments[id];
		info.name = QString::fromLocal8Bit(pInstrument->InstrumentName);
		info.exchangeId = QString(pInstrument->ExchangeID);
		info.deadline = QDate::fromString(QString(pInstrument->ExpireDate), "yyyyMMdd");
		info.marginRate = pInstrument->LongMarginRatio;
		info.multiplier = pInstrument->VolumeMultiple;
		info.minimumUnit = pInstrument->PriceTick;

		if (bIsLast)
		{
			ee->putEvent(Event(EVENT_CONTRACT));
		}
	}
}

///请求查询合约手续费率响应
void CtpTdApi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (!is_error_rsp(pRspInfo) && pInstrumentCommissionRate)
	{
		// 获取数据
		QMap <QString, InstrumentInfo>&	allInstruments = de->allInstruments;

		// 此处只返回合约类型
		QString id = pInstrumentCommissionRate->InstrumentID;
		// 将合约中所有该类型的合约取出
		QStringList id_list;
		foreach(auto inst, allInstruments)
		{
			if (inst.id.contains(id))
			{
				id_list << inst.id;
			}
		}

		// 统一赋值费率
		foreach(auto inst, id_list)
		{
			if (allInstruments.find(inst) != allInstruments.end())
			{
				//如果存在
			}
			else
			{
				InstrumentInfo instrumentInfo;
				instrumentInfo.id = inst;
				allInstruments.insert(inst, instrumentInfo);
			}

			//写入内存对象中
			InstrumentInfo& info = allInstruments[inst];
			double &oc = pInstrumentCommissionRate->OpenRatioByVolume;
			double &oc_rate = pInstrumentCommissionRate->OpenRatioByMoney;
			double &cc = pInstrumentCommissionRate->OpenRatioByVolume;
			double &cc_rate = pInstrumentCommissionRate->CloseRatioByMoney;
			double &today_cc = pInstrumentCommissionRate->CloseTodayRatioByVolume;
			double &today_cc_rate = pInstrumentCommissionRate->CloseTodayRatioByMoney;
			info.openCommission = (oc > oc_rate ? oc : oc_rate);
			info.closeCommission = (cc > cc_rate ? cc : cc_rate);
			info.closeTodayCommission = (today_cc > today_cc_rate ? today_cc : today_cc_rate);
		}
		
		if (bIsLast)
		{
			ee->putEvent(Event(EVENT_CONTRACT));
		}
	}
}

bool CtpTdApi::get_is_td_connect()
{
	return is_td_connect.load();
}

bool CtpTdApi::get_is_td_logout()
{
	return is_td_logout.load();
}

void CtpTdApi::ctp_td_init(QString tdAddress, QString userid, QString password, QString brokerid)
{
	// 保存基本信息
	strncpy_s(usr_td_info.brokerid, brokerid.toStdString().c_str(), sizeof(usr_td_info.brokerid));
	strncpy_s(usr_td_info.userid, userid.toStdString().c_str(), sizeof(usr_td_info.userid));
	strncpy_s(usr_td_info.password, password.toStdString().c_str(), sizeof(usr_td_info.password));

	QString con_path = "conn_file/" + userid + "/td/";

	QDir temp;
	if (!temp.exists(con_path))
	{
		bool n = temp.mkpath(con_path);
	}

	char frontaddress[512] = { 0 };
	strncpy_s(frontaddress, tdAddress.toStdString().c_str(), sizeof(frontaddress));

	TdApi = CThostFtdcTraderApi::CreateFtdcTraderApi(con_path.toStdString().c_str());
	TdApi->RegisterSpi(this);

	//订阅共有流、私有流
	TdApi->SubscribePublicTopic(THOST_TERT_RESTART);
	TdApi->SubscribePrivateTopic(THOST_TERT_RESTART);

	//注册并连接前置机
	TdApi->RegisterFront(frontaddress);
	TdApi->Init();

	//开启请求队列
	QueryQueue.cmd_begin();
	TradeQueue.cmd_begin();
}

void CtpTdApi::ctp_td_release()
{
	//清空请求队列
	QueryQueue.cmd_stop();
	TradeQueue.cmd_stop();

	if (TdApi != nullptr)
	{
		TdApi->RegisterSpi(nullptr);
		TdApi->Release();
		TdApi = nullptr;
	}
}

// 每6秒分别查一次资金和持仓
void CtpTdApi::ctp_td_query(Event ev)
{
	query_count += 1;

	if (query_count > query_trgger)
	{
		query_count = 0;

		if (0 == query_function_index)
		{
			ctp_td_getAccount();
			query_function_index += 1;
		}
		else
		{
			ctp_td_getPosition();
			query_function_index = 0;
		}
	}
}

void CtpTdApi::ctp_td_login()
{
	/*连接成功后开始登录*/
	CThostFtdcReqUserLoginField loginField = { 0 };
	strncpy_s(loginField.BrokerID, usr_td_info.brokerid, sizeof(loginField.BrokerID));
	strncpy_s(loginField.UserID, usr_td_info.userid, sizeof(loginField.UserID));
	strncpy_s(loginField.Password, usr_td_info.password, sizeof(loginField.Password));

	//把指令放到队列尾部,下面各条指令的执行方法类似
	std::shared_ptr<CtpCommand> command = std::make_shared<LoginCommand>(TdApi, loginField, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}

void CtpTdApi::ctp_td_logout()
{
	CThostFtdcUserLogoutField logoutField = { 0 };
	strncpy_s(logoutField.BrokerID, usr_td_info.brokerid, sizeof(logoutField.BrokerID));
	strncpy_s(logoutField.UserID, usr_td_info.userid, sizeof(logoutField.BrokerID));

	std::shared_ptr<CtpCommand> command = std::make_shared<LogoutCommand>(TdApi, logoutField, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}

void CtpTdApi::ctp_td_getSettlement()
{
	CThostFtdcSettlementInfoConfirmField comfirmField = { 0 };
	strncpy_s(comfirmField.BrokerID, usr_td_info.brokerid, sizeof(comfirmField.BrokerID));
	strncpy_s(comfirmField.InvestorID, usr_td_info.userid, sizeof(comfirmField.BrokerID));

	std::shared_ptr<CtpCommand> command = std::make_shared<ComfirmSettlementCommand>(TdApi, comfirmField, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}
void CtpTdApi::ctp_td_getInstrument()
{
	//查询合约基本信息
	CThostFtdcQryInstrumentField qre_instr = { 0 };
	strncpy_s(qre_instr.ExchangeID, "", sizeof(qre_instr.ExchangeID));
	usr_td_info.requestID++;
	TdApi->ReqQryInstrument(&qre_instr, usr_td_info.requestID);
	std::this_thread::sleep_for(std::chrono::milliseconds(1100));	//受流限制，每次查询间隔1秒

	// 查询合约费率：这部分有问题，始终返回-2，只能在合约查询完成后，即last==true时查询费率。
	// 同时，合约费率只能依合约查询，那每次查询都需要睡1秒，肯定不能在程序启动时全部查询，只能在策略中加载交易合约费率
	// ctp_td_getCommission();
}

void CtpTdApi::ctp_td_getCommission(QString ins_id)
{
	//查询合约手续费
	CThostFtdcQryInstrumentCommissionRateField qry_com = { 0 };
	strncpy_s(qry_com.BrokerID, usr_td_info.brokerid, sizeof(qry_com.BrokerID));
	strncpy_s(qry_com.InvestorID, usr_td_info.userid, sizeof(qry_com.InvestorID));
	strncpy_s(qry_com.InstrumentID, ins_id.toStdString().c_str(), sizeof(qry_com.InstrumentID));

	std::shared_ptr<CtpCommand> command = std::make_shared<InstrumentCommissionCommand>(TdApi, qry_com, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}

void CtpTdApi::ctp_td_getAccount()
{
	CThostFtdcQryTradingAccountField accountField = { 0 };
	strncpy_s(accountField.BrokerID, usr_td_info.brokerid, sizeof(accountField.BrokerID));
	strncpy_s(accountField.InvestorID, usr_td_info.userid, sizeof(accountField.BrokerID));

	std::shared_ptr<CtpCommand> command = std::make_shared<QueryFundCommand>(TdApi, accountField, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}

void CtpTdApi::ctp_td_getPosition()
{
	CThostFtdcQryInvestorPositionField accountField = { 0 };
	strncpy_s(accountField.BrokerID, usr_td_info.brokerid, sizeof(accountField.BrokerID));
	strncpy_s(accountField.InvestorID, usr_td_info.userid, sizeof(accountField.BrokerID));
	
	std::shared_ptr<CtpCommand> command = std::make_shared<QueryPositionCommand>(TdApi, accountField, usr_td_info.requestID);
	QueryQueue.addCommand(command);
}
QString CtpTdApi::ctp_td_send_limitOrder(TThostFtdcInstrumentIDType instrumentid, TThostFtdcPriceType price, TThostFtdcVolumeType volume,
	TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset)
{
	CThostFtdcInputOrderField orderField = { 0 };
	
	strncpy_s(orderField.InstrumentID, instrumentid, sizeof(orderField.InstrumentID));
	orderField.OrderPriceType = THOST_FTDC_OPT_LimitPrice;		//限价
	orderField.Direction = direction;
	orderField.CombOffsetFlag[0] = offset;
	orderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//投机 
	orderField.LimitPrice = price;
	orderField.VolumeTotalOriginal = volume;		//数量
	orderField.TimeCondition = THOST_FTDC_TC_GFD;				//当日有效 '3'
	orderField.VolumeCondition = THOST_FTDC_VC_AV;				//任何数量 '1'
	orderField.ContingentCondition = THOST_FTDC_CC_Immediately;	//立即触发'1'

	return ctp_td_order_insert(orderField);
}

QString CtpTdApi::ctp_td_send_marketOrder(TThostFtdcInstrumentIDType instrumentid, TThostFtdcVolumeType volume, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset)
{
	CThostFtdcInputOrderField orderField = { 0 };

	strncpy_s(orderField.InstrumentID, instrumentid, sizeof(orderField.InstrumentID));
	orderField.OrderPriceType = THOST_FTDC_OPT_AnyPrice;		//市价
	orderField.Direction = direction;
	orderField.CombOffsetFlag[0] = offset;
	orderField.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//投机 
	orderField.VolumeTotalOriginal = volume;		//数量
	orderField.TimeCondition = THOST_FTDC_TC_IOC;				//立即完成 '1'
	orderField.VolumeCondition = THOST_FTDC_VC_AV;				//任何数量 '1'
	orderField.ContingentCondition = THOST_FTDC_CC_Immediately;	//立即触发'1'

	return ctp_td_order_insert(orderField);
}

// 三组都可以用来撤单
//InstrumentID + FrontID + SessionID + OrderRef
//ExchangeID + TraderID + OrderLocalID
//ExchangeID + OrderSysID
void CtpTdApi::ctp_td_cancelOrder(TThostFtdcInstrumentIDType instrumentID, TThostFtdcExchangeIDType exchangeID, TThostFtdcOrderRefType orderID, TThostFtdcFrontIDType frontID, TThostFtdcSessionIDType sessionID)
{
	//设置撤单信息
	CThostFtdcInputOrderActionField orderField = { 0 };
	strncpy_s(orderField.InstrumentID, instrumentID, sizeof(orderField.InstrumentID));
	strncpy_s(orderField.ExchangeID, exchangeID, sizeof(orderField.BrokerID));
	strncpy_s(orderField.OrderRef, orderID, sizeof(orderField.OrderRef));
	orderField.FrontID = frontID;
	orderField.SessionID = sessionID;

	orderField.ActionFlag = THOST_FTDC_AF_Delete;	//删除报单 '0'
	strncpy_s(orderField.BrokerID, usr_td_info.brokerid, sizeof(orderField.BrokerID));
	strncpy_s(orderField.InvestorID, usr_td_info.userid, sizeof(orderField.InvestorID));
	
	std::shared_ptr<CtpCommand> command = std::make_shared<WithdrawOrderCommand>(TdApi, orderField, usr_td_info.requestID);
	TradeQueue.addCommand(command);
}

//////////////////////////////////////////////////////////////////////////

bool CtpTdApi::is_error_rsp(CThostFtdcRspInfoField *pRspInfo)
{
	return (pRspInfo && (pRspInfo->ErrorID != 0));
}

QString CtpTdApi::ctp_td_order_insert(CThostFtdcInputOrderField& orderField)
{
	if (!is_td_tradable.load())
	{
		// 结算单未确认(所有qDebug的地方输出到日志监控模块)
		qDebug() << "Miss Settlement Confirmation";
		return "Error";
	}

	strncpy_s(orderField.BrokerID, usr_td_info.brokerid, sizeof(orderField.BrokerID));
	strncpy_s(orderField.InvestorID, usr_td_info.userid, sizeof(orderField.InvestorID));

	_snprintf_s(orderField.OrderRef, sizeof(orderField.OrderRef), _TRUNCATE, "%012d", ++usr_td_info.maxOrderRef);

	orderField.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//非强平 '0'
	orderField.IsAutoSuspend = 0;
	orderField.UserForceClose = 0;

	// 增加一个任务到队列
	std::shared_ptr<CtpCommand> command = std::make_shared<InsertOrderCommand>(TdApi, orderField, usr_td_info.requestID);
	TradeQueue.addCommand(command);

	return orderField.OrderRef;
}

void CtpTdApi::ctp_td_order_update(OrderInfo& order)
{
	// 获取数据
	QMap<QString, OrderInfo>& allOrderDict = de->allOrderDict;
	QMap<QString, OrderInfo>& workingOrderDict = de->workingOrderDict;

	// 填充日内所有委托
	QString ordID = order.orderID;
	if (allOrderDict.find(ordID) != allOrderDict.end())
	{
		// 如果存在
		allOrderDict[ordID] = order;
	}
	else
	{
		// 如果不存在
		allOrderDict.insert(ordID, order);
	}
	// 修改活动委托
	QChar status = order.status;
	auto working_find_iter = workingOrderDict.find(ordID);
	if (working_find_iter != workingOrderDict.end())
	{
		if (status == THOST_FTDC_OST_AllTraded || status == THOST_FTDC_OST_Canceled)
		{
			workingOrderDict.erase(working_find_iter);
		}
	}
	else
	{
		workingOrderDict.insert(ordID, order);
	}
}