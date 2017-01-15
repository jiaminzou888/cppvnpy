#include "tableView_Position.h"
#include <QApplication>
#include "MainEngine.h"

extern MainEngine *me;
/********************TableModel********************/

TableModelPosition::TableModelPosition(QObject *parent)
	: QAbstractTableModel(parent)
{
}
void TableModelPosition::setHorizontalHeaderList(QStringList horizontalHeaderList)
{
	horizontal_header_list = horizontalHeaderList;
}
void TableModelPosition::setModalDatas(QList<PositionInfo> *rowlist)
{
	data_list = rowlist;
}

void TableModelPosition::refrushModel()
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
int TableModelPosition::rowCount(const QModelIndex & /* parent */) const
{
	return data_list ? data_list->size() : 0;
}
//返回列数
int TableModelPosition::columnCount(const QModelIndex & /* parent */) const
{
	return horizontal_header_list.size();
}

//返回一个项的任意角色的值，这个项被指定为QModelIndex
QVariant TableModelPosition::data(const QModelIndex &index, int role) const
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
			return data_list->at(index.row()).directName;
		}
		if (index.column() == 3)
		{
			return data_list->at(index.row()).position;
		}
		if (index.column() == 4)
		{
			return data_list->at(index.row()).ydPosition;
		}
		if (index.column() == 5)
		{
			return data_list->at(index.row()).frozen;
		}
		if (index.column() == 6)
		{
			return data_list->at(index.row()).price;
		}
		if (index.column() == 7)
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
QVariant TableModelPosition::headerData(int section, Qt::Orientation orientation, int role) const
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
TableView_Position::TableView_Position(QWidget *parent)
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
	
	this->header << QStringLiteral("合约代码") << QStringLiteral("名称") << QStringLiteral("方向")
				 << QStringLiteral("持仓量") << QStringLiteral("昨持仓") << QStringLiteral("冻结量")
				 << QStringLiteral("价格") << QStringLiteral("接口");

	model = new TableModelPosition();
	model->setModalDatas(&grid_data_list);
	model->setHorizontalHeaderList(header);
	
	this->setModel(model);

	// 刷新用户持仓信息表格
	me->register_event(EVENT_POSITION_UI, this, &TableView_Position::updateData);
}

TableView_Position::~TableView_Position(void)
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
void TableView_Position::updateData(Event ev)
{
	initData(ev);
	model->refrushModel();
}
void TableView_Position::initData(Event ev)
{
	grid_data_list.clear();

	QMap <QString, PositionInfo>&& all_position = me->me_getPositionInfo();

	QMap <QString, PositionInfo>::iterator it;
	for (it = all_position.begin(); it != all_position.end(); ++it)
	{
		grid_data_list.append(*it);
	}
}