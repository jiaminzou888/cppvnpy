#ifndef TABLEVIEW_INSTRUMENTINFO_
#define TABLEVIEW_INSTRUMENTINFO_

#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>

#include "ctpapi/ThostFtdcMdApi.h"
#include "Model/PublicStruct.h"
#include "EventBase.h"


/********************TableModel********************/
class TableModelInstrumentInfo : public QAbstractTableModel
{
public:
	TableModelInstrumentInfo(QObject *parent = 0);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setModalDatas(QList<InstrumentInfo> *rowlist);
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

	QList<InstrumentInfo> *data_list;
	//QMap<QString, double> currencyMap;
};


// 合约监控模块
/********************ReadOnlyTableView********************/
class TableView_InstrumentInfo : public QTableView
{
	Q_OBJECT

public:
	TableView_InstrumentInfo(QWidget *parent = 0);
	~TableView_InstrumentInfo(void);

public slots:
	//更新表格
	void updateData(Event ev);

private:

	void initData(Event ev);
	TableModelInstrumentInfo *model;
	QStringList header;

	QList<InstrumentInfo> grid_data_list;
	//ProgressBarDelegate *progressbar_delegate;
	//更新table
	QTimer *updateTimer;

};
#endif