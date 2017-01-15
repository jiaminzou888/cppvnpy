#include "tableView_Order.h"
#include <QApplication>
#include "MainEngine.h"

extern MainEngine *me;
/********************TableModel********************/

TableModelOrder::TableModelOrder(QObject *parent)
	: QAbstractTableModel(parent)
{
}
void TableModelOrder::setHorizontalHeaderList(QStringList horizontalHeaderList)
{
	horizontal_header_list = horizontalHeaderList;
}
void TableModelOrder::setModalDatas(QList<OrderInfo> *rowlist)
{
	data_list = rowlist;
}

void TableModelOrder::refrushModel()
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
int TableModelOrder::rowCount(const QModelIndex & /* parent */) const
{
	return data_list ? data_list->size() : 0;
}
//返回列数
int TableModelOrder::columnCount(const QModelIndex & /* parent */) const
{
	return horizontal_header_list.size();
}

//返回一个项的任意角色的值，这个项被指定为QModelIndex
QVariant TableModelOrder::data(const QModelIndex &index, int role) const
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
			return data_list->at(index.row()).orderID;
		}
		if (index.column() == 1)
		{
			return data_list->at(index.row()).symbol;
		}
		if (index.column() == 2)
		{
			return data_list->at(index.row()).symbol;
		}
		if (index.column() == 3)
		{
			return data_list->at(index.row()).direction;
		}
		if (index.column() == 4)
		{
			return data_list->at(index.row()).offset;
		}
		if (index.column() == 5)
		{
			return data_list->at(index.row()).price;
		}
		if (index.column() == 6)
		{
			return data_list->at(index.row()).totalVolume;
		}
		if (index.column() == 7)
		{
			return data_list->at(index.row()).tradeVolume;
		}
		if (index.column() == 8)
		{
			return data_list->at(index.row()).status;
		}
		if (index.column() == 9)
		{
			return data_list->at(index.row()).orderTime;
		}
		if (index.column() == 10)
		{
			return data_list->at(index.row()).cancelTime;
		}
		if (index.column() == 11)
		{
			return data_list->at(index.row()).frontID;
		}
		if (index.column() == 12)
		{
			return data_list->at(index.row()).sessionID;
		}
		if (index.column() == 13)
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
QVariant TableModelOrder::headerData(int section, Qt::Orientation orientation, int role) const
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
TableView_Order::TableView_Order(QWidget *parent)
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
	
	this->header << QStringLiteral("委托编号") << QStringLiteral("合约代码") << QStringLiteral("名称")
				 << QStringLiteral("方向") << QStringLiteral("开平") << QStringLiteral("价格")
				 << QStringLiteral("委托数量") << QStringLiteral("成交数量") << QStringLiteral("状态")
				 << QStringLiteral("委托时间") << QStringLiteral("撤销时间") << QStringLiteral("前置编号")
				 << QStringLiteral("会话编号") << QStringLiteral("接口");

	model = new TableModelOrder();
	model->setModalDatas(&grid_data_list);
	model->setHorizontalHeaderList(header);
	
	this->setModel(model);

	// 刷新用户持仓信息表格
	me->register_event(EVENT_ORDER, this, &TableView_Order::updateData);
}

TableView_Order::~TableView_Order(void)
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
void TableView_Order::updateData(Event ev)
{
	initData(ev);
	model->refrushModel();
}
void TableView_Order::initData(Event ev)
{
	grid_data_list.clear();

	QMap <QString, OrderInfo>&& all_order = me->me_getOrderInfo();

	QMap <QString, OrderInfo>::iterator it;
	for (it = all_order.begin(); it != all_order.end(); ++it)
	{
		grid_data_list.append(*it);
	}
}