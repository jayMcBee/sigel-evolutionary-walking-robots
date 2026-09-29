#include "MT_GUI/MT_ExperimentItem.h"

#include <QPixmap>
#include <QIcon>

MT_ExperimentItem::MT_ExperimentItem(QTreeWidget *parent, int pos, QString title, const QPixmap &pix) : QTreeWidgetItem()
{
	position = pos;
	setText(0, title);
	setIcon(0, QIcon(pix));
	parent->insertTopLevelItem(0, this);
}

int MT_ExperimentItem::getPos()
{
	return position;
}