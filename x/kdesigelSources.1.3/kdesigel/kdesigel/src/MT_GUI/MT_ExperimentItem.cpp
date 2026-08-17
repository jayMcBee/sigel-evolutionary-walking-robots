// MT_ExperimentItem.cpp: Implementierung der Klasse MT_ExperimentItem.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_ExperimentItem.h"

#include <qpixmap.h>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_ExperimentItem::MT_ExperimentItem(QListView *parent, int pos, QString title, QPixmap &pix) : QListViewItem(parent)
{
	position = pos;
	setText(0, title);
	setPixmap(0, pix);
}

MT_ExperimentItem::~MT_ExperimentItem()
{

}

int MT_ExperimentItem::getPos()
{
	return position;
}