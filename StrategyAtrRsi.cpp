#include "StrategyAtrRsi.h"

#include <QDebug>

#include "CtaEngine.h"
#include "GLogWrapper.h"

StrategyAtrRsi::StrategyAtrRsi(CtaEngine* ce, std::string name, std::string symbol)
: StrategyBase(ce, name, symbol)
{
	// 测试功能
	//////////////////////////////////////////////////////////////////////////
	atrLength = 5;
	atrMaLength = 5;
	rsiLength = 5;
	rsiEntry = 10;
	trailingPercent = 0.8;


	bufferSize = 10;
	//////////////////////////////////////////////////////////////////////////


	// 初始化策略参数
	highArray.reserve(bufferSize);
	lowArray.reserve(bufferSize);
	closeArray.reserve(bufferSize);
}

void StrategyAtrRsi::onInit()
{
	// 初始化RSI入场阈值
	rsiBuy = 50 + rsiEntry;
	rsiSell = 50 - rsiEntry;

	/*
	// 载入历史数据，并采用回放计算的方式初始化策略数值
	std::vector<CtaBarData> hist_bars;
	loadBar(initDays, hist_bars);
	for_each(hist_bars.begin(), hist_bars.end(), [](const CtaBarData& bar)
	{

	});
	*/

	// putEvent();
}

void StrategyAtrRsi::onStart()
{
	// putEvent();
}

void StrategyAtrRsi::onStop()
{
	// putEvent();
}

void StrategyAtrRsi::onTick(QuoteInfo& quote)
{
	// 计算K线
	int tick_time = convert_time_str2int(quote.time.toStdString().c_str());
	int tickMinute = tick_time / 100;
	if (tickMinute != barMinute)
	{
		if (!bar.is_new)
		{
			// 跳表时推送旧bar,初始化新bar
			onBar(bar);
		}

		// 赋值构造函数重置旧bar
		bar = CtaBarData();
		bar.vtSymbol = quote.vtSymbol.toStdString();
		bar.symbol = quote.symbol.toStdString();
		//bar.exchange = 
		
		// 初始化OHLC
		bar.open = quote.lastPrice;
		bar.high = quote.lastPrice;
		bar.low = quote.lastPrice;
		bar.close = quote.lastPrice;

		bar.time = tick_time;	// K线的时间设为第一个Tick的时间
		barMinute = tickMinute; // 更新当前的分钟
	}
	else
	{
		double last_price = quote.lastPrice;
		bar.high = (bar.high > last_price) ? bar.high : last_price;
		bar.low = (bar.low < last_price) ? bar.low : last_price;
		bar.close = last_price;
		bar.is_new = false;
	}
}

void StrategyAtrRsi::onBar(CtaBarData& bar)
{
	// 撤销之前发出的尚未成交的委托（包括限价单和停止单）：（限价单好像只在CtaEngine中维护吧？）
	// 新Bar到来时，前一个Bar发出的停止单均不作数
	for_each(orderList.begin(), orderList.end(), [this](const std::string& orderID)
	{
		cancelOrder(orderID);
	});
	orderList.clear();

	// 保存K线数据
	if (bufferCount == bufferSize)
	{
		highArray.erase(highArray.begin());
		lowArray.erase(lowArray.begin());
		closeArray.erase(closeArray.begin());
		bufferCount--;
	}

	highArray.push_back(bar.high);
	lowArray.push_back(bar.low);
	closeArray.push_back(bar.close);
	bufferCount++;

	// 继续累积原始K线
	if (bufferCount < bufferSize)
	{
		return;
	}

	// 计算指标数值

	// ATR
	int atr_size = -1;
	std::vector<double> atr_vec;
	if (!TechIndicator::ATR(highArray, lowArray, closeArray, atrLength, atr_size, atr_vec))
	{
		// 错误日志
		return;
	}
	double atrValue = atr_vec[atr_size-1];

	if (atrCount == bufferSize)
	{
		atrArray.erase(atrArray.begin());
		atrCount--;
	}

	atrArray.push_back(atrValue); 
	atrCount++;

	// 继续累积atr数据
	if (atrCount < bufferSize)
	{
		return;
	}

	// MA
	int atr_ma_size = -1;
	std::vector<double> atr_ma_vec;
	if (!TechIndicator::MA(atrArray, atrMaLength, atr_ma_size, atr_ma_vec))
	{
		// 错误日志
		return;
	}
	// ATR的均线
	double atrMa = atr_ma_vec[atr_ma_size - 1];

	// RSI
	int rsi_size = -1;
	std::vector<double> rsi_vec;
	if (!TechIndicator::RSI(closeArray, rsiLength, rsi_size, rsi_vec))
	{
		// 错误日志
		return;
	}

	// 策略核心指标
	double rsiValue = rsi_vec[rsi_size-1];

	// 判断是否要进行交易
	//char sz_log[256] = { 0 };
	//sprintf_s(sz_log, "ATR: %.4f; ATR_MA: %.4f; RSI: %.4f", atrValue, atrMa, rsiValue);
	//qDebug() << sz_log;


	// 当前无仓位
	if (0 == pos)
	{
		// 持仓周期内的高低价，无仓位时初始化
		intraTradeHigh = bar.high;
		intraTradeLow = bar.low;

		// ATR数值上穿其移动平均线，说明行情短期内波动加大
		// 即处于趋势的概率较大，适合CTA开仓
		if (atrValue > atrMa)
		{
			// 使用RSI指标的趋势行情时，会在超买超卖区钝化特征，作为开仓信号
			if (rsiValue > rsiBuy)
			{
				// 这里为了保证成交，选择超价5个整指数点下单
				buy(bar.close+5, 1);

				char log_msg[512] = { 0 };
				_snprintf_s(log_msg, sizeof(log_msg), _TRUNCATE, "Open: BUY: rsiValue: %lf, atrMa: %lf, rsiValue: %lf ... close: %lf  ", atrValue, atrMa, rsiValue, bar.close+5);
				GLOG(log_msg, CGLog::CGLog_INFO);
			}
			else if (rsiValue < rsiSell)
			{
				short_(bar.close-5, 1);

				char log_msg[512] = { 0 };
				_snprintf_s(log_msg, sizeof(log_msg), _TRUNCATE, "Open: SHORT: rsiValue: %lf, atrMa: %lf, rsiValue: %lf ... close: %lf  ", atrValue, atrMa, rsiValue, bar.close-5);
				GLOG(log_msg, CGLog::CGLog_INFO);
			}
		}
	}
	// 持有多头（注意学习此处停止单的逻辑！其实止盈也是某种意义上的止损！！！）
	else if (pos >= 1)
	{
		// 计算多头持有期内的最高价，以及重置最低价
		intraTradeHigh = (intraTradeHigh > bar.high) ? intraTradeHigh : bar.high;
		intraTradeLow = bar.low;
		// 计算多头移动止损: 止损百分比不动，随价格变动而移动
		double longStop = intraTradeHigh * (1 - trailingPercent / 100.);

		// 发出本地止损委托，并且把委托号记录下来，用于后续撤单(不断发出止损委托)
		std::string orderID = sell(longStop, 1, true);
		orderList.push_back(orderID);	// orderList 只在OnBar中push_back和clear

		char log_msg[512] = { 0 };
		_snprintf_s(log_msg, sizeof(log_msg), _TRUNCATE, "Stop: SELL: intraTradeHigh: %lf, longStop: %lf, orderID: %s  ", intraTradeHigh, longStop, orderID.c_str());
		GLOG(log_msg, CGLog::CGLog_INFO);
	}
	// 持有空头
	else if (pos <= -1)
	{
		intraTradeLow = (intraTradeLow < bar.low) ? intraTradeLow : bar.low;
		intraTradeHigh = bar.high;
		// 计算空头移动止损: 止损百分比不动，随价格变动而移动
		double shortStop = intraTradeLow * (1 + trailingPercent / 100.);
		// 计算多头移动止损: 止损百分比不动，随价格变动而移动
		std::string orderID = cover(shortStop, 1, true);
		orderList.push_back(orderID);  // orderList 只在OnBar中push_back和clear

		char log_msg[512] = { 0 };
		_snprintf_s(log_msg, sizeof(log_msg), _TRUNCATE, "Stop: COVER: intraTradeLow: %lf, shortStop: %lf, orderID: %s  ", intraTradeLow, shortStop, orderID.c_str());
		GLOG(log_msg, CGLog::CGLog_INFO);
	}

	// putEvent();
}

void StrategyAtrRsi::onOrder(OrderInfo& order)
{
	
}

void StrategyAtrRsi::onTrade(TradeInfo& trade)
{
	char log_msg[512] = { 0 };
	_snprintf_s(log_msg, sizeof(log_msg), _TRUNCATE, "onTrade: orderID: %s, volume: %d, tradeTime: %s, stgPos: %d  ", trade.orderID.toStdString().c_str(), trade.volume, trade.tradeTime.toStdString().c_str(), pos);
	GLOG(log_msg, CGLog::CGLog_INFO);
}

