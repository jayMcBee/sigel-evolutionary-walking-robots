// MT_PopListViewItem.h: interface for class MT_PopListViewItem.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_)
#define AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_

#include <QTreeWidget>
#include "MT_GPSystem/MT_Individual.h"

class MT_PopListViewItem : public QTreeWidgetItem  
{
public:
	MT_PopListViewItem(QTreeWidget *parent);
	MT_PopListViewItem(QTreeWidget *parent, MT_Individual *ind);
	virtual ~MT_PopListViewItem();
	void setPos(int NewPos);
	int getPos();

private:
	int position;	// position in the population; does not necessarily correspond
					// the position in the listView
};

#endif // !defined(AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_)
