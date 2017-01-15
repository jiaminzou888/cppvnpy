#include "tableView_Quote.h"
#include <QApplication>
#include <QElapsedTimer>

#include "MainEngine.h"

extern MainEngine *me;
/********************TableModel********************/

TableModelQuote::TableModelQuote(QObject *parent)
	: QAbstractTableModel(parent)
{
}
void TableModelQuote::setHorizontalHeaderList(QStringList horizontalHeaderList)
{
	horizontal_header_list = horizontalHeaderList;
}
void TableModelQuote::setModalDatas(QList<QuoteInfo> *rowlist)
{
	data_list = rowlist;
}

void TableModelQuote::refrushModel()
{
	beginResetModel();
	endResetModel();

	//emit updateCount(this->rowCount(QModelIndex()));
}
//void TableModel::setCurrencyMap(const QMap<QString, double> &map)
//{
//	currencyMap = map;
//	//重置模型至原始状态，告诉所有视图，他们数据都无效，强制刷新数据
//	//reset();
//}

//返回行数
int TableModelQuote::rowCount(const QModelIndex & /* parent */) const
{
	return data_list ? data_list->size() : 0;
}
//返回列数
int TableModelQuote::columnCount(const QModelIndex & /* parent */) const
{
	return horizontal_header_list.size();
}

//返回一个项的任意角色的值，这个项被指定为QModelIndex
QVariant TableModelQuote::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}
		
	if (role == Qt::TextAlignmentRole) 
	{
		return int(Qt::AlignCenter | Qt::AlignVCenter);
	}
	else if (role == Qt::DisplayRole) 
	{

		if (index.column() == 0)
		{
			return data_list->at(index.row()).symbol;
		}
		if (index.column() == 1)
		{
			return data_list->at(index.row()).vtSymbol;
		}
		if (index.column() == 2)
		{
			return data_list->at(index.row()).lastPrice;
		}
		if (index.column() == 3)
		{
			return data_list->at(index.row()).preClosePrice;
		}
		if (index.column() == 4)
		{
			return data_list->at(index.row()).volume;
		}
		if (index.column() == 5)
		{
			return data_list->at(index.row()).openInterest;
		}
		if (index.column() == 6)
		{
			return data_list->at(index.row()).openPrice;
		}
		if (index.column() == 7)
		{
			return data_list->at(index.row()).highPrice;
		}
		if (index.column() == 8)
		{
			return data_list->at(index.row()).lowPrice;
		}
		if (index.column() == 9)
		{
			return data_list->at(index.row()).bidPrice1;
		}
		if (index.column() == 10)
		{
			return data_list->at(index.row()).bidVolume1;
		}
		if (index.column() == 11)
		{
			return data_list->at(index.row()).askPrice1;
		}
		if (index.column() == 12)
		{
			return data_list->at(index.row()).askVolume1;
		}
		if (index.column() == 13)
		{
			return data_list->at(index.row()).time;
		}
		if (index.column() == 14)
		{
			return data_list->at(index.row()).gatewayName;
		}

		//QString rowCurrency = currencyAt(index.row());
		//QString columnCurrency = currencyAt(index.column());

		//if (currencyMap.value(rowCurrency) == 0.0)
		//	return "####";

		//double amount = currencyMap.value(columnCurrency)
		//	/ currencyMap.value(rowCurrency);

		//return QString("%1").arg(amount, 0, 'f', 4);
	}
	return QVariant();
}
//返回表头名称,(行号或列号，水平或垂直，角色)
QVariant TableModelQuote::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) // 水平表头  
		{
			return (horizontal_header_list.size() > section) ? horizontal_header_list[section] : QVariant();
		}
		else
		{
			// return (vertical_header_list.size() > section) ? vertical_header_list[section] : QVariant();
		}
	}

	return QVariant();
}
//获取当前关键字
//QString TableModel::currencyAt(int offset) const
//{
//	return (currencyMap.begin() + offset).key();
//}

/********************TableView********************/
TableView_Quote::TableView_Quote(QWidget *parent)
	: QTableView(parent)
{

	//行背景色交替改变
	this->setAlternatingRowColors(true);
	//前景色 背景色
	//this->setStyleSheet("QTableView{background-color: rgb(250, 250, 115);"
	//	"alternate-background-color: rgb(141, 163, 215);}");
	//整行选中的方式
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->horizontalHeader()->setStretchLastSection(true);
	this->horizontalHeader()->setHighlightSections(false);
	this->verticalHeader()->setVisible(false);
	this->setShowGrid(true);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	this->header << QStringLiteral("合约代码") << QStringLiteral("名称") << QStringLiteral("最新价")
				 << QStringLiteral("昨收盘价") << QStringLiteral("成交量") << QStringLiteral("持仓量")
				 << QStringLiteral("开盘价") << QStringLiteral("最高价") << QStringLiteral("最低价")
				 << QStringLiteral("买一价") << QStringLiteral("买一量") << QStringLiteral("卖一价")
				 << QStringLiteral("卖一量") << QStringLiteral("时间") << QStringLiteral("接口");

	model = new TableModelQuote();
	model->setModalDatas(&grid_data_list);
	model->setHorizontalHeaderList(header);
	
	this->setModel(model);

	// MV模式中定时器自动刷新行情报价
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(false);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateData()));
	updateTimer->start(3000); // 页面行情报价3秒快照刷新
}

TableView_Quote::~TableView_Quote(void)
{
	//if (progressbar_delegate) {
	//	delete progressbar_delegate;
	//	progressbar_delegate = NULL;
	//}

	if (model) 
	{
		delete model;
		model = NULL;
	}
}
void TableView_Quote::updateData()
{
	initData();
	model->refrushModel();
}
void TableView_Quote::initData()
{
	grid_data_list.clear();

	QMap <QString, CThostFtdcDepthMarketDataField>&& market_set = me->me_getLastMarketData();

	QMap <QString, CThostFtdcDepthMarketDataField>::iterator it;
	for (it = market_set.begin(); it != market_set.end(); ++it)
	{
		QuoteInfo qm;
		qm.symbol = (it->InstrumentID);
		//qm.vtSymbol = (me->getInstrumentInfo()[it->InstrumentID].getName());
		qm.lastPrice = (it->LastPrice);
		qm.preClosePrice = (it->PreClosePrice);
		qm.volume = (it->Volume);
		qm.openInterest = (it->OpenInterest);
		qm.openPrice = (it->OpenPrice);
		qm.highPrice = (it->HighestPrice);
		qm.lowPrice = (it->LowestPrice);
		qm.bidPrice1 = (it->BidPrice1);
		qm.bidVolume1 = (it->BidVolume1);
		qm.askPrice1 = (it->AskPrice1);
		qm.askVolume1 = (it->AskVolume1);
		//qm.setTime(QString(QLatin1String(it->UpdateTime) + "%1%2").arg(":").arg(QString::number(it->UpdateMillisec)));
		qm.time = (it->UpdateTime);
		qm.gatewayName = ("CTP");

		grid_data_list.append(qm);
	}
}