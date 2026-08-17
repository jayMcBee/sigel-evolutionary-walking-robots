// MT_PopListViewItem.h: Schnittstelle für die Klasse MT_PopListViewItem.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_)
#define AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_

#include <qlistview.h>
#include "MT_GPSystem/MT_Individual.h"

class MT_PopListViewItem : public QListViewItem  
{
public:
	MT_PopListViewItem(QListView *parent);
	MT_PopListViewItem(QListView *parent, MT_Individual *ind);
	virtual ~MT_PopListViewItem();
	void setPos(int NewPos);
	int getPos();

private:
	int position;	// position in the population; does not necessarily correspond
					// the position in the listView
};

#endif // !defined(AFX_MT_POPLISTVIEWITEM_H__2BF5C50C_726A_4564_90DB_3E9CA925E011__INCLUDED_)
