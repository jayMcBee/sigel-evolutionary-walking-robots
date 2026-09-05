// MT_PopListViewItem.cpp: implementation of class MT_PopListViewItem.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_PopListViewItem.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

// Prepends, like Qt 2's QListViewItem(QListView*). See MT_ExperimentItem.cpp
// for why the parent is not handed to the base class.
MT_PopListViewItem::MT_PopListViewItem(QTreeWidget *parent) : QTreeWidgetItem()
{ 
	position = -1;
	parent->insertTopLevelItem(0, this);
}

MT_PopListViewItem::MT_PopListViewItem(QTreeWidget *parent, MT_Individual *ind) : QTreeWidgetItem()
{
	position = ind->getPosition();
	setText(0, QString::number(ind->getName()) );
	setText(1, QString::number(ind->getFitness()) );
	setText(2, QString::number(ind->getAge()) );
	setText(3, QString::number(ind->getProgram()->getLength()) );
	parent->insertTopLevelItem(0, this);
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