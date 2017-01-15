#ifndef TABLEVIEW_POSITION_
#define TABLEVIEW_POSITION_


#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>

#include "Model/PositionBuffer.h"
#include "EventBase.h"

/********************TableModel********************/
class TableModelPosition : public QAbstractTableModel
{
public:
	TableModelPosition(QObject *parent = 0);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setModalDatas(QList<PositionInfo> *rowlist);
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

	QList<PositionInfo> *data_list;
	//QMap<QString, double> currencyMap;
};

// 持仓监控模块
/********************ReadOnlyTableView********************/
class TableView_Position : public QTableView
{
	Q_OBJECT

public:

	TableView_Position(QWidget *parent = 0);
	~TableView_Position(void);

public slots:
	//更新表格
	void updateData(Event ev);

private:

	void initData(Event ev);
	TableModelPosition *model;
	QStringList header;

	QList<PositionInfo> grid_data_list;
	//ProgressBarDelegate *progressbar_delegate;
	//更新table
	QTimer *updateTimer;

};
#endif // !TABLEVIEW_POSITION_