// MT_ExperimentItem.h: interface for class MT_ExperimentItem.
//
//////////////////////////////////////////////////////////////////////

#ifndef MT_GUI_MT_EXPERIMENTITEM_H
#define MT_GUI_MT_EXPERIMENTITEM_H

#include <QTreeWidget>
#include <QPixmap>
#include <QString>

class MT_ExperimentItem : public QTreeWidgetItem  
{
public:
	MT_ExperimentItem(QTreeWidget *parent, int pos, QString title, const QPixmap &pix);

	int getPos();

private:
	int position;
};

#endif // MT_GUI_MT_EXPERIMENTITEM_H
