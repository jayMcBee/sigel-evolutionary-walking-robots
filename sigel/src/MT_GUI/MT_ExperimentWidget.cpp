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
#include "MT_GUI/MT_ExperimentWidget.h"
#include "MT_GUI/MT_ExperimentItem.h"

#include <QTreeWidget>
#include <QPixmap>
#include <stdlib.h>

MT_ExperimentWidget::MT_ExperimentWidget(QWidget* parent)
	: QTreeWidget( parent )
{
	// Sorting stays off. Each item goes on top of the list, so the pages show
	// in reverse order: Strategy first, Statistics last.
	setSortingEnabled(false);
	clear();
	setColumnCount(1);
	setHeaderLabels(QStringList("Meta experiment"));

	QString pixPath = std::getenv("SIGEL_ROOT");
	pixPath.append("/pixmaps/");
	
	new MT_ExperimentItem(this, 5, "Statistics", QPixmap( pixPath+"mt_StatSmall.xpm" ));
	new MT_ExperimentItem(this, 4, "Selection", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 3, "GP Parameters", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 2, "Population", QPixmap( pixPath+"individualSmall.xpm" ));
	new MT_ExperimentItem(this, 1, "Individual",QPixmap( pixPath+"mt_InstrSmall.xpm" ));
	new MT_ExperimentItem(this, 0, "Strategy",QPixmap( pixPath+"mt_StrategySmall.xpm" ));

	// before showing the window for the first time select the first item
	// in the list
	setCurrentItem(topLevelItem(0));
}

void MT_ExperimentWidget::slotCurrentChanged(QTreeWidgetItem *newSelection)
{
	// remember the previously selected item to allow reselection
	if(!newSelection)
		return;
	prevSelectedItems.enqueue(static_cast<MT_ExperimentItem*>(newSelection));
	if(prevSelectedItems.count() > 2)
		prevSelectedItems.dequeue();
}

void MT_ExperimentWidget::lastSelected()
{
	// head() on an empty queue is undefined and segfaults. Reached on the
	// first refused page switch.
	if(prevSelectedItems.isEmpty())
		return;
	MT_ExperimentItem *stickingItem = prevSelectedItems.head();
	setCurrentItem(stickingItem);
	prevSelectedItems.enqueue(stickingItem);
	prevSelectedItems.dequeue();
}
