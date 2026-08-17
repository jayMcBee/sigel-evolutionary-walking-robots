// MT_PopListViewItem.cpp: Implementierung der Klasse MT_PopListViewItem.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_PopListViewItem.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_PopListViewItem::MT_PopListViewItem(QListView *parent) : QListViewItem(parent)
{ 
	position = -1;
}

MT_PopListViewItem::MT_PopListViewItem(QListView *parent, MT_Individual *ind) : QListViewItem(parent)
{
	position = ind->getPosition();
	setText(0, QString::number(ind->getName()) );
	setText(1, QString::number(ind->getFitness()) );
	setText(2, QString::number(ind->getAge()) );
	setText(3, QString::number(ind->getProgram()->getLength()) );
}

MT_PopListViewItem::~MT_PopListViewItem()
{}

int MT_PopListViewItem::getPos()
{
	return position;
}

void MT_PopListViewItem::setPos(int NewPos)
{
	 position = NewPos;
}