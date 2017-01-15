#include "CtpMdApi.h"

#include <QDir>
#include <QDebug>
#include <string>

void CtpMdApi::OnFrontConnected()
{
	is_md_connect.store(true);
}

void CtpMdApi::OnFrontDisconnected(int nReason)
{
	is_md_connect.store(false);
}

void CtpMdApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo->ErrorID == 0)
	{
		// 登录成功
		qDebug() << "MD Succeed With Login";
	}
	else
	{

	}
}

void CtpMdApi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo->ErrorID == 0)
	{
		// 退出标记
		is_md_logout.store(true);

		// 登出成功
		qDebug() << "MD  Succeed With  Logout";
	}
	else
	{

	}
}

void CtpMdApi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	// 仅供UI定时刷新展示(Per 3 Seconds)
	QString id = pDepthMarketData->InstrumentID;
	if (de->lastMarketDataSet.find(id) != de->lastMarketDataSet.end())
	{
		//如果存在，则更新
		de->lastMarketDataSet[id] = *pDepthMarketData;
	}
	else
	{
		de->lastMarketDataSet.insert(id, *pDepthMarketData);
	}

	// 推送策略Tick事件
	Event tickEvent(EVENT_TICK);
	tickEvent.data_ = std::move(std::shared_ptr<char>((char*)new QuoteInfo));
	QuoteInfo* quote = (QuoteInfo*)tickEvent.data_.get();

	quote->symbol = (pDepthMarketData->InstrumentID);
	quote->vtSymbol = (quote->symbol);

	quote->upperLimit = (pDepthMarketData->UpperLimitPrice);
	quote->lowerLimit = (pDepthMarketData->LowerLimitPrice);

	quote->openPrice = (pDepthMarketData->OpenPrice);
	quote->highPrice = (pDepthMarketData->HighestPrice);
	quote->lowPrice = (pDepthMarketData->LowestPrice);
	quote->lastPrice = (pDepthMarketData->LastPrice);

	quote->bidPrice1 = (pDepthMarketData->BidPrice1);
	quote->bidVolume1 = (pDepthMarketData->BidVolume1);
	quote->askPrice1 = (pDepthMarketData->AskPrice1);
	quote->askVolume1 = (pDepthMarketData->AskVolume1);

	quote->volume = (pDepthMarketData->Volume);
	quote->openInterest = (pDepthMarketData->OpenInterest);
	quote->preClosePrice = (pDepthMarketData->PreClosePrice);

	// quote->setTime(QString(QLatin1String(pDepthMarketData->UpdateTime) + "%1%2").arg(":").arg(QString::number(pDepthMarketData->UpdateMillisec)));
	quote->time = (pDepthMarketData->UpdateTime);

	ee->putEvent(tickEvent);

	//qDebug() << "contractName: " << pDepthMarketData->InstrumentID << "lastPrice: " << pDepthMarketData->LastPrice << "updateTime: " << pDepthMarketData->UpdateTime;
}

bool CtpMdApi::get_is_md_connect()
{
	return is_md_connect.load();
}

bool CtpMdApi::get_is_md_logout()
{
	return is_md_logout.load();
}

void CtpMdApi::ctp_md_init(QString mdaddress)
{
	QString con_path = "conn_file/md/";

	QDir temp;
	if (!temp.exists(con_path))
	{
		bool n = temp.mkpath(con_path);
	}
	char frontaddress[512] = { 0 };
	strncpy_s(frontaddress, mdaddress.toStdString().c_str(), sizeof(frontaddress));

	MdApi = CThostFtdcMdApi::CreateFtdcMdApi(con_path.toStdString().c_str());
	MdApi->RegisterSpi(this);
	MdApi->RegisterFront(frontaddress);
	MdApi->Init();
}

void CtpMdApi::ctp_md_release()
{
	if (MdApi != nullptr)
	{
		MdApi->RegisterSpi(nullptr);
		MdApi->Release();
		MdApi = nullptr;
	}
}

void CtpMdApi::ctp_md_login()
{
	CThostFtdcReqUserLoginField loginField = { 0 };
	MdApi->ReqUserLogin(&loginField, requestID++);
}

void CtpMdApi::ctp_md_logout()
{
	CThostFtdcUserLogoutField logoutField = { 0 };
	MdApi->ReqUserLogout(&logoutField, requestID++);
}

void CtpMdApi::ctp_md_subscribe(QString instrumentid)
{
	char id[15] = { 0 };
	strncpy_s(id, instrumentid.toStdString().c_str(), sizeof(id));

	char* insts[] = { id };
	MdApi->SubscribeMarketData(insts, 1);
}