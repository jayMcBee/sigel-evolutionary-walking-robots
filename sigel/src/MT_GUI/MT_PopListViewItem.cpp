/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "MT_GUI/MT_PopListViewItem.h"

// Inserts the item at the top of the list. QTreeWidgetItem(parent) would
// append it, so the parent is not handed to the base class.
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

int MT_PopListViewItem::getPos()
{
	return position;
}

void MT_PopListViewItem::setPos(int NewPos)
{
	 position = NewPos;
}