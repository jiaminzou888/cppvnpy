#pragma once

#include <string>
#include <memory>

#define EVENT_LOG "eLog"                    //# 日志事件，通常使用某个监听函数直接显示
#define EVENT_TDLOGIN "eTdLogin"            //# 交易服务器登录成功事件

#define EVENT_TIMER "eTimer"                //# 计时器事件，每隔1秒发送一次

#define EVENT_CONTRACT  "eContract"         // # 合约查询回报事件
#define EVENT_INVESTOR  "eInvestor"         // # 投资者查询回报事件
#define EVENT_ACCOUNT   "eAccount"          // # 账户查询回报事件

#define EVENT_POSITION	  "ePosition"       // # 持仓查询回报事件
#define EVENT_POSITION_UI "ePositionUI"     // # 持仓查询回报事件（更新UI）

#define EVENT_TICK "eTick"					//# 行情推送事件
#define EVENT_TICK_CONTRACT "eTick."		//# 特定合约的行情事件

#define EVENT_ORDER "eOrder"                //# 报单推送事件
#define EVENT_ORDER_ORDERREF "eOrder."      //# 特定报单号的报单事件

#define EVENT_TRADE "eTrade"                //# 成交推送事件
#define EVENT_TRADE_CONTRACT "eTrade."      //# 特定合约的成交事件

//////////////////////////////////////////////////////////////////////////
#define EVENT_STG_BEG "eBegStg"             //# CTA策略开启事件
#define EVENT_STG_END "eEngStg"				//# CTA策略关闭事件

// 将Event传到每一个回调函数中，data_包含堆上的数据指针，仿vnpy，在回调函数内部将char* data转换为需要的数据类型
// 后期可以使用tc_malloc将所有的new截取到内存池中，提升性能的同时避免内存碎片
struct Event
{
	Event() = default;
	Event(std::string type) : type_(type){}

	std::string type_;
	std::shared_ptr<char> data_;
};