#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_maintrade.h"

#include "MainEngine.h"

#include "TableView/tableView_Account.h"
#include "TableView/tableView_Quote.h"
#include "TableView/tableView_Position.h"
#include "TableView/tableView_InstrumentInfo.h"
#include "TableView/tableView_Order.h"

#include "qt_basictool/CompleteLineEdit.h"

extern MainEngine* me;

class MainTrade : public QMainWindow
{
	Q_OBJECT

public:
	MainTrade(QWidget *parent = 0);

	bool eventFilter(QObject *, QEvent *) override;

private:
	Ui::MainTradeClass ui;

	// 界面暂时全量刷新，后期可优化为只更新事件推送的那一条记录
	TableView_Account* tableView_account;
	TableView_Position* tableView_position;
	TableView_Quote* tableView_quote;
	TableView_InstrumentInfo* tableView_instrumentInfo;
	TableView_Order* tableView_order;

	// 索引编辑框
	CompleteLineEdit* order_contract_edit;

private Q_SLOTS:
	void OnInstrmentLineTextSubscribe();
	void OnSendOrderClicked();
	void OnCancelAll();

	void OnStartCTA();
	void OnStopCTA();
private:
	bool getSendOrderRequest(orderCommonRequest& order_field);
};
