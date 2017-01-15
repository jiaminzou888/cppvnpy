#ifndef TABLEVIEW_ACCOUNT_
#define TABLEVIEW_ACCOUNT_


#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>

#include "Model/PublicStruct.h"
#include "EventBase.h"

/********************TableModel********************/
class TableModelAccount : public QAbstractTableModel
{
public:
	TableModelAccount(QObject *parent = 0);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setModalDatas(QList<AccountInfo> *rowlist);
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

	QList<AccountInfo> *data_list;
	//QMap<QString, double> currencyMap;
};

// 资金账户监控模块
/********************ReadOnlyTableView********************/
class TableView_Account : public QTableView
{
	Q_OBJECT

public:

	TableView_Account(QWidget *parent = 0);
	~TableView_Account(void);

public slots:
	//更新表格
	void updateData(Event ev);

private:
	void initData(Event ev);
	TableModelAccount *model;
	QStringList header;

	QList<AccountInfo> grid_data_list;
	//ProgressBarDelegate *progressbar_delegate;
	//更新table
	QTimer *updateTimer;

};
#endif // !TABLEVIEW_ACCOUNT_