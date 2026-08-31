// MT_ExperimentItem.h: Schnittstelle für die Klasse MT_ExperimentItem.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_EXPERIMENTITEM_H__F6ADF263_E73B_40EA_B14B_7D9BEB6D2AAB__INCLUDED_)
#define AFX_MT_EXPERIMENTITEM_H__F6ADF263_E73B_40EA_B14B_7D9BEB6D2AAB__INCLUDED_

#include <QTreeWidget>
#include <QPixmap>
#include <QString>

class MT_ExperimentItem : public QTreeWidgetItem  
{
public:
	MT_ExperimentItem(QTreeWidget *parent, int pos, QString title, const QPixmap &pix);
	virtual ~MT_ExperimentItem();

	int getPos();

private:
	int position;
};

#endif // !defined(AFX_MT_EXPERIMENTITEM_H__F6ADF263_E73B_40EA_B14B_7D9BEB6D2AAB__INCLUDED_)
