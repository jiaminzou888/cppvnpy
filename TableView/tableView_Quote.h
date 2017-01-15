#ifndef TABLEVIEW_QUOTE_
#define TABLEVIEW_QUOTE_

#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>

#include "Model/PublicStruct.h"
#include "EventBase.h"

/********************TableModel********************/
class TableModelQuote : public QAbstractTableModel
{
public:
	TableModelQuote(QObject *parent = 0);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setModalDatas(QList<QuoteInfo> *rowlist);
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

	QList<QuoteInfo> *data_list;
	//QMap<QString, double> currencyMap;
};


// 行情报价监控模块
/********************ReadOnlyTableView********************/
class TableView_Quote : public QTableView
{
	Q_OBJECT

public:

	TableView_Quote(QWidget *parent = 0);
	~TableView_Quote(void);

public slots:
	//更新表格
	void updateData();

private:

	void initData();
	TableModelQuote *model;
	QStringList header;

	QList<QuoteInfo> grid_data_list;
	//ProgressBarDelegate *progressbar_delegate;

	//更新table
	QTimer *updateTimer;

};
#endif