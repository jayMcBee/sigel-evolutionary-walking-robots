// MT_ExperimentItem.cpp: implementation of class MT_ExperimentItem.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_ExperimentItem.h"

#include <QPixmap>
#include <QIcon>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

// Qt 2's QListViewItem(QListView*) PREPENDED -- it forwarded to
// QListViewItem::insertItem, which is head insertion (qlistview.cpp:585).
// Qt 6's QTreeWidgetItem(QTreeWidget*) appends, so the parent is NOT passed to
// the base and the item is inserted at 0 by hand. MT_ExperimentWidget builds
// its six pages 5..0 with sorting off and then selects firstChild(), so an
// appending port would both draw and select the wrong page.
// `QPixmap &' was a non-const reference bound to a temporary at all six
// call sites -- a GCC extension in 2003, rejected today. const changes no
// behaviour and is the minimum that compiles.
MT_ExperimentItem::MT_ExperimentItem(QTreeWidget *parent, int pos, QString title, const QPixmap &pix) : QTreeWidgetItem()
{
	position = pos;
	setText(0, title);
	setIcon(0, QIcon(pix));
	parent->insertTopLevelItem(0, this);
}

MT_ExperimentItem::~MT_ExperimentItem()
{

}

int MT_ExperimentItem::getPos()
{
	return position;
}