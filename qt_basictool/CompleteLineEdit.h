#pragma once

#include <QtWidgets/QLineEdit>
#include <QStringList> 

class QListView;  
class QStringListModel;  
class QModelIndex;  

class CompleteLineEdit : public QLineEdit 
{
    Q_OBJECT  
public:  
    CompleteLineEdit(QWidget *parent = 0);
	
public slots:
	// 动态的显示完成列表  
    void setCompleter(const QString &text);
	// 点击完成列表中的项，使用此项自动完成输入的单词
    void completeText(const QModelIndex &index);

protected:  
    virtual void keyPressEvent(QKeyEvent *e);  
    virtual void focusOutEvent(QFocusEvent *e);  

private:  
    QStringList words; // 整个完成列表的单词  
    QListView* listView; // 完成列表  
    QStringListModel* model; // 完成列表的model  
};