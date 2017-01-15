#include "maintrade.h"
#include "QMessageBox"


MainTrade::MainTrade(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// UI监控模块
	tableView_account = new TableView_Account;
	ui.horizontalLayout_5->addWidget(tableView_account);

	tableView_position = new TableView_Position;
	ui.horizontalLayout_7->addWidget(tableView_position);

	tableView_quote = new TableView_Quote;
	ui.horizontalLayout_6->addWidget(tableView_quote);

	tableView_instrumentInfo = new TableView_InstrumentInfo;
	ui.horizontalLayout_8->addWidget(tableView_instrumentInfo);

	tableView_order = new TableView_Order;
	ui.horizontalLayout_10->addWidget(tableView_order);

	// 行情订阅
	order_contract_edit = new CompleteLineEdit(ui.groupBox_orderPanel);
	order_contract_edit->setGeometry(78, 20, 113, 20);

	ui.scrollArea_instruQuote->setMinimumHeight(120);
	connect(order_contract_edit, SIGNAL(returnPressed()), this, SLOT(OnInstrmentLineTextSubscribe()));	// 行情订阅槽

	// 用户登录
	ui.pushButton_login->installEventFilter(this);
	ui.comboBox_address->addItem("SimNow", 0);
	ui.comboBox_address->setItemText(0, QStringLiteral("上期SimNow"));

	ui.lineEdit_userID->clear();
	ui.lineEdit_userID->setText("011800");

	ui.lineEdit_password->clear();
	ui.lineEdit_password->setText("49446532");

	// 用户登出
	ui.pushButton_logout->installEventFilter(this);

	// 下单面板
	ui.doubleSpinBox_price->setMaximum(1000000.00);
	ui.spinBox_volume->setMaximum(1000000);

	ui.radioButton_buy->setChecked(true);
	ui.radioButton_closeBefore->setChecked(true);

	connect(ui.pushButton_sendOrder, SIGNAL(clicked()), this, SLOT(OnSendOrderClicked()));	// 下单槽
	connect(ui.pushButton_cancelOrder, SIGNAL(clicked()), this, SLOT(OnCancelAll()));		// 撤单槽

	// CTA测试面板
	connect(ui.pushButton_startCTA, SIGNAL(clicked()), this, SLOT(OnStartCTA()));	// 启动槽
	connect(ui.pushButton_stopCTA, SIGNAL(clicked()), this, SLOT(OnStopCTA()));		// 关闭槽
}

// 这是事件过滤器似乎没有额外的作用，看上去使用信号槽相应登录/退出更优
bool MainTrade::eventFilter(QObject *target, QEvent *e)
{
	if (target == ui.pushButton_login)
	{
		if (e->type() == QEvent::MouseButtonRelease)
		{
			QString userid = ui.lineEdit_userID->text();
			QString psd = ui.lineEdit_password->text();
			if (userid.isEmpty() || psd.isEmpty())
			{
				return false;
			}

			ui.pushButton_login->setEnabled(false);
			ui.pushButton_logout->setEnabled(true);

			ui.lineEdit_userID->setEnabled(false);
			ui.lineEdit_password->setEnabled(false);

			me->me_login(userid, psd, "9999", "tcp://180.168.146.187:10010", "tcp://180.168.146.187:10000");
			//me->me_login(userid, psd, "9999", "tcp://180.168.146.187:10031", "tcp://180.168.146.187:10030");  //7*24
			std::this_thread::sleep_for(std::chrono::milliseconds(300));

			return true;
		}
		else
		{
			return false;
		}
	}
	else if (target == ui.pushButton_logout)
	{
		if (e->type() == QEvent::MouseButtonRelease)
		{
			ui.pushButton_login->setEnabled(true);
			ui.pushButton_logout->setEnabled(false);

			ui.lineEdit_userID->setEnabled(true);
			ui.lineEdit_password->setEnabled(true);

			me->me_logout();
			std::this_thread::sleep_for(std::chrono::milliseconds(300));

			// 关闭程序
			close();

			return true;
		}
		else
		{
			return false;
		}
	}

	return QMainWindow::eventFilter(target, e);
}

void MainTrade::OnInstrmentLineTextSubscribe()
{
	me->me_subscribe(order_contract_edit->text());
}

void MainTrade::OnSendOrderClicked()
{
	// 下单: 默认手动下单为限价单，其他单（包括市价、中金所高级市价、大商所止盈止损以及FAK/FOK等只在策略中调用）
	// 按道理，每发一笔单就得在本地维护一个委托单列表
	if (me->me_get_is_login())
	{
		orderCommonRequest order_field;
		if (getSendOrderRequest(order_field))
		{
			me->me_sendDefaultOrder(order_field);
		}
	}
}

void MainTrade::OnCancelAll()
{
	QMap<QString, OrderInfo>&& working_orders = me->me_getWorkingOrderInfo();

	for (auto it = working_orders.begin(); it != working_orders.end(); ++it)
	{
		cancelCommonRequest cancel_field;
		strncpy_s(cancel_field.instrument, it->symbol.toStdString().c_str(), sizeof(cancel_field.instrument));
		strncpy_s(cancel_field.exchange, it->exchange.toStdString().c_str(), sizeof(cancel_field.exchange));
		strncpy_s(cancel_field.order_ref, it->orderID.toStdString().c_str(), sizeof(cancel_field.order_ref));
		cancel_field.front_id = it->frontID;
		cancel_field.session_id = it->sessionID;
		me->me_cancelOrder(cancel_field);
	}
}

void MainTrade::OnStartCTA()
{
	if (me->me_get_is_login())
	{
		me->me_strat_cta();
	}
}

void MainTrade::OnStopCTA()
{
	if (me->me_get_is_login())
	{
		me->me_stop_cta();
	}
}

bool MainTrade::getSendOrderRequest(orderCommonRequest& order_field)
{
	bool get_ret = true;

	// instrument_id
	strncpy_s(order_field.instrument, order_contract_edit->text().toStdString().c_str(), sizeof(order_field.instrument) - 1);
	// price
	order_field.price = ui.doubleSpinBox_price->text().toDouble();
	// volume
	order_field.volume = ui.spinBox_volume->text().toInt();

	// direction
	if (ui.radioButton_buy->isChecked())
	{
		order_field.direction = THOST_FTDC_D_Buy;
	}
	else if (ui.radioButton_sell->isChecked())
	{
		order_field.direction = THOST_FTDC_D_Sell;
	}
	else
	{
		order_field.direction = 'a';
	}

	// offset
	if (ui.radioButton_open->isChecked())
	{
		order_field.offset = THOST_FTDC_OF_Open;
	}
	else if (ui.radioButton_close->isChecked())
	{
		order_field.offset = THOST_FTDC_OF_Close;
	}
	else if (ui.radioButton_closeToday->isChecked())
	{
		order_field.offset = THOST_FTDC_OF_CloseToday;
	}
	else if (ui.radioButton_closeBefore->isChecked())
	{
		order_field.offset = THOST_FTDC_OF_CloseYesterday;
	}
	else
	{
		order_field.offset = 'a';
	}

	// 怎么判断合约为空
	if ('a' == order_field.direction || 'a' == order_field.offset 
		|| abs(order_field.price - 0.0) < 0.000001 || 0 == order_field.volume)
	{
		get_ret = false;
	}

	return get_ret;
}