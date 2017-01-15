#pragma once

#include <qthread.h>
#include <qtimer.h>
#include <memory>
#include <thread>
#include <chrono>

#include "cpp_basictool/CppQueue.hpp"
#include "ctpapi/ThostFtdcTraderApi.h"

//封装ctp命令的接口
class CtpCommand
{
public:
	virtual ~CtpCommand()
	{
		api = nullptr;
	}
	virtual int execute() = 0;

protected:
	CtpCommand::CtpCommand(int& requestID, CThostFtdcTraderApi *api)
		:requestID(requestID)
	{
		this->api = api;
		requestID++;
	}

	int& requestID;
	CThostFtdcTraderApi* api;
};
/*登录*/
class LoginCommand :public CtpCommand
{
public:
	LoginCommand::LoginCommand(CThostFtdcTraderApi *api, CThostFtdcReqUserLoginField& loginField, int &requestID)
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->loginField), sizeof(CThostFtdcReqUserLoginField), &loginField, sizeof(CThostFtdcReqUserLoginField));
	}
	int LoginCommand::execute()
	{
		return api->ReqUserLogin(&loginField, requestID);
	}
private:
	CThostFtdcReqUserLoginField loginField;
};
/*登出*/
class LogoutCommand : public CtpCommand
{
public:
	LogoutCommand::LogoutCommand(CThostFtdcTraderApi *api, CThostFtdcUserLogoutField& logoutField, int &requestID)
		: CtpCommand(requestID, api)
	{
		memcpy_s(&(this->logoutField), sizeof(CThostFtdcUserLogoutField), &logoutField, sizeof(CThostFtdcUserLogoutField));
	}
	int LogoutCommand::execute()
	{
		return api->ReqUserLogout(&logoutField, requestID);
	}
private:
	CThostFtdcUserLogoutField logoutField;
};
/*确认结算结果*/
class ComfirmSettlementCommand :public CtpCommand
{
public:
	ComfirmSettlementCommand::ComfirmSettlementCommand(CThostFtdcTraderApi *api, CThostFtdcSettlementInfoConfirmField& comfirmField, int &requestID) 
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->comfirmField), sizeof(CThostFtdcSettlementInfoConfirmField), &comfirmField, sizeof(CThostFtdcSettlementInfoConfirmField));
	}
	int ComfirmSettlementCommand::execute()
	{
		return api->ReqSettlementInfoConfirm(&comfirmField, requestID);
	}
private:
	CThostFtdcSettlementInfoConfirmField comfirmField;
};
/*查询资金*/
class QueryFundCommand :public CtpCommand
{
public:
	QueryFundCommand::QueryFundCommand(CThostFtdcTraderApi *api, CThostFtdcQryTradingAccountField& accountField,int &requestID)
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->accountField), sizeof(CThostFtdcQryTradingAccountField), &accountField, sizeof(CThostFtdcQryTradingAccountField));
	}
	int QueryFundCommand::execute()
	{
		return api->ReqQryTradingAccount(&accountField, requestID);
	}
private:
	CThostFtdcQryTradingAccountField accountField;
};
/*查询持仓*/
class QueryPositionCommand :public CtpCommand
{
public:
	QueryPositionCommand::QueryPositionCommand(CThostFtdcTraderApi *api, CThostFtdcQryInvestorPositionField& accountField,int &requestID) 
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->accountField), sizeof(CThostFtdcQryInvestorPositionField), &accountField, sizeof(CThostFtdcQryInvestorPositionField));
	}
	int QueryPositionCommand::execute()
	{
		return api->ReqQryInvestorPosition(&accountField, requestID);
	}
private:
	CThostFtdcQryInvestorPositionField accountField;
};
/*报单请求*/
class InsertOrderCommand :public CtpCommand
{
public:
	InsertOrderCommand::InsertOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderField& orderField,int &requestID) 
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->orderField), sizeof(CThostFtdcInputOrderField), &orderField, sizeof(CThostFtdcInputOrderField));
	}
	int InsertOrderCommand::execute()
	{
		return api->ReqOrderInsert(&orderField, requestID);
	}
private:
	CThostFtdcInputOrderField orderField;
};
/*撤单请求*/
class WithdrawOrderCommand :public CtpCommand
{
public:
	WithdrawOrderCommand::WithdrawOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderActionField& orderField,int &requestID) 
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->orderField), sizeof(CThostFtdcInputOrderActionField), &orderField, sizeof(CThostFtdcInputOrderActionField));
	}
	int WithdrawOrderCommand::execute()
	{
		return api->ReqOrderAction(&orderField, requestID);
	}
private:
	CThostFtdcInputOrderActionField orderField;
};
/*合约费率查询请求*/
class InstrumentCommissionCommand :public CtpCommand
{
public:
	InstrumentCommissionCommand::InstrumentCommissionCommand(CThostFtdcTraderApi *api, CThostFtdcQryInstrumentCommissionRateField& comField, int &requestID)
		:CtpCommand(requestID, api)
	{
		memcpy_s(&(this->comField), sizeof(CThostFtdcQryInstrumentCommissionRateField), &comField, sizeof(CThostFtdcQryInstrumentCommissionRateField));
	}
	int InstrumentCommissionCommand::execute()
	{
		return api->ReqQryInstrumentCommissionRate(&comField, requestID);
	}
private:
	CThostFtdcQryInstrumentCommissionRateField comField;
};

//指令队列
class CommandQueue :public QThread
{
	Q_OBJECT
public:

	CommandQueue(int time) :sleep_time(time) {}
	~CommandQueue() = default;

	void cmd_begin()
	{
		// 开启队列开关
		cmd_active.store(true);

		// 启动线程
		thread_active.store(true);
		start();
	}

	void cmd_stop()
	{
		// 关闭队列开关，并等待任务执行完毕
		cmd_active.store(false);
		while (!commandQueue.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		}

		// 退出线程
		thread_active.store(false);
		commandQueue.notify_all(); // 通知等待在空队列上的线程

		quit();
		wait();
	}

	void CommandQueue::addCommand(std::shared_ptr<CtpCommand> newCommand)
	{
		if (cmd_active.load())
		{
			commandQueue.push_back(newCommand);
		}
	}

private:
	ConcurrentQueue<std::shared_ptr<CtpCommand>> commandQueue;

	int  sleep_time{ 0 };
	std::atomic<bool> thread_active{ false };
	std::atomic<bool> cmd_active{ false };

private:
	void CommandQueue::cut_in_command(std::shared_ptr<CtpCommand> newCommand)
	{
		if (cmd_active.load())
		{
			commandQueue.push_front(newCommand);
		}
	}

	void CommandQueue::run()
	{
		while (thread_active.load())
		{
			// 获取任务
			std::shared_ptr<CtpCommand> command;
			if (commandQueue.wait_and_pop(command, -1))
			{
				if (command->execute())
				{
					// 调试很长时间导致断线重连后，点击退出按钮，发现查询队列数根本下不去，导致程序始终无法正常退出关闭，后续要查一下是什么原因。
					cut_in_command(command);
				}
	
				// 发送指令成功,休息一秒（CTP要求）
				std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
			}
		}
	}
};