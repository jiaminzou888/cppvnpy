#include "tableView_InstrumentInfo.h"
#include <QApplication>
#include "MainEngine.h"


extern MainEngine *me;
/********************TableModel********************/

TableModelInstrumentInfo::TableModelInstrumentInfo(QObject *parent)
	: QAbstractTableModel(parent)
{
}
void TableModelInstrumentInfo::setHorizontalHeaderList(QStringList horizontalHeaderList)
{
	horizontal_header_list = horizontalHeaderList;
}
void TableModelInstrumentInfo::setModalDatas(QList<InstrumentInfo> *rowlist)
{
	data_list = rowlist;
}

void TableModelInstrumentInfo::refrushModel()
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
int TableModelInstrumentInfo::rowCount(const QModelIndex & /* parent */) const
{
	return data_list ? data_list->size() : 0;
}
//返回列数
int TableModelInstrumentInfo::columnCount(const QModelIndex & /* parent */) const
{
	return horizontal_header_list.size();
}

//返回一个项的任意角色的值，这个项被指定为QModelIndex
QVariant TableModelInstrumentInfo::data(const QModelIndex &index, int role) const
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
			return data_list->at(index.row()).id;
		}
		if (index.column() == 1)
		{
			return data_list->at(index.row()).name;
		}
		if (index.column() == 2)
		{
			return data_list->at(index.row()).exchangeId;
		}
		if (index.column() == 3)
		{
			return data_list->at(index.row()).deadline;
		}
		if (index.column() == 4)
		{
			return data_list->at(index.row()).marginRate;
		}
		if (index.column() == 5)
		{
			return data_list->at(index.row()).multiplier;
		}
		if (index.column() == 6)
		{
			return data_list->at(index.row()).openCommission;
		}
		if (index.column() == 7)
		{
			return data_list->at(index.row()).closeCommission;
		}
		if (index.column() == 8)
		{
			return data_list->at(index.row()).closeTodayCommission;
		}
		if (index.column() == 9)
		{
			return data_list->at(index.row()).minimumUnit;
		}
		if (index.column() == 10)
		{
			return data_list->at(index.row()).tradable;
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
QVariant TableModelInstrumentInfo::headerData(int section, Qt::Orientation orientation, int role) const
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
TableView_InstrumentInfo::TableView_InstrumentInfo(QWidget *parent)
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

	this->header << QStringLiteral("合约代码") << QStringLiteral("名称") << QStringLiteral("交易所")
				 << QStringLiteral("最后交割日") << QStringLiteral("保证金率") << QStringLiteral("合约乘数")
				 << QStringLiteral("开仓手续费") << QStringLiteral("平仓手续费")
				 << QStringLiteral("平今手续费") << QStringLiteral("最小变动单位")
				 << QStringLiteral("是否可以交易");


	model = new TableModelInstrumentInfo();
	model->setModalDatas(&grid_data_list);
	model->setHorizontalHeaderList(header);
	
	this->setModel(model);

	// 更新合约窗口事件
	me->register_event(EVENT_CONTRACT, this, &TableView_InstrumentInfo::updateData);
}

TableView_InstrumentInfo::~TableView_InstrumentInfo(void)
{
	//if (progressbar_delegate) {
	//	delete progressbar_delegate;
	//	progressbar_delegate = NULL;
	//}

	if (model) 
	{
		delete model;
		model = nullptr;
	}
}
void TableView_InstrumentInfo::updateData(Event ev)
{
	initData(ev);
	model->refrushModel();
}
void TableView_InstrumentInfo::initData(Event ev)
{
	grid_data_list.clear();

	QMap <QString, InstrumentInfo>&& all_contracts = me->me_getInstrumentInfo();

	QMap <QString, InstrumentInfo>::iterator it;
	for (it = all_contracts.begin(); it != all_contracts.end(); ++it)
	{
		InstrumentInfo qm;
		qm.id = (it->id);
		qm.name = (it->name);
		qm.exchangeId = (it->exchangeId);
		qm.deadline = (it->deadline);
		qm.marginRate = (it->marginRate);
		qm.multiplier = (it->multiplier);
		qm.openCommission = (it->openCommission);
		qm.closeCommission = (it->closeCommission);
		qm.closeTodayCommission = (it->closeTodayCommission);
		qm.minimumUnit = (it->minimumUnit);
		qm.tradable = (it->tradable);
		
		grid_data_list.append(qm);
	}
}