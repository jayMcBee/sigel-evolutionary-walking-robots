#ifndef MT_GUI_MT_POPLISTVIEWITEM_H
#define MT_GUI_MT_POPLISTVIEWITEM_H

#include <QTreeWidget>
#include "MT_GPSystem/MT_Individual.h"

class MT_PopListViewItem : public QTreeWidgetItem  
{
public:
	MT_PopListViewItem(QTreeWidget *parent);
	MT_PopListViewItem(QTreeWidget *parent, MT_Individual *ind);
	void setPos(int NewPos);
	int getPos();

private:
	int position;	// position in the population; does not necessarily correspond
					// the position in the listView
};

#endif // MT_GUI_MT_POPLISTVIEWITEM_H
