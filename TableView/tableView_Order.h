#ifndef TABLEVIEW_ORDER_
#define TABLEVIEW_ORDER_


#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>

#include "Model/PublicStruct.h"
#include "EventBase.h"

/********************TableModel********************/
class TableModelOrder : public QAbstractTableModel
{
public:
	TableModelOrder(QObject *parent = 0);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setModalDatas(QList<OrderInfo> *rowlist);
	void refrushModel();

	//void setCurrencyMap(const QMap<QString, double> &map);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
	//QString currencyAt(int offset) const;
	QStringList horizontal_header_list;
	//QStringList vertical_header_list;

	QList<OrderInfo> *data_list;
	//QMap<QString, double> currencyMap;
};

// 持仓监控模块
/********************ReadOnlyTableView********************/
class TableView_Order : public QTableView
{
	Q_OBJECT

public:
	TableView_Order(QWidget *parent = 0);
	~TableView_Order(void);

public slots:
	//更新表格
	void updateData(Event ev);

private:

	void initData(Event ev);
	TableModelOrder *model;
	QStringList header;

	QList<OrderInfo> grid_data_list;
	//ProgressBarDelegate *progressbar_delegate;
	//更新table
	QTimer *updateTimer;

};
#endif // !TABLEVIEW_POSITION_