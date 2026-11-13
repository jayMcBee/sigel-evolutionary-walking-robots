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
#ifndef MT_GUI_POPULATIONWIDGET
#define MT_GUI_POPULATIONWIDGET

#include "MT_GUI/MT_PopulationWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/MT_PopListViewItem.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_GPManager.h"

#include <QAction>
#include <QToolBar>
#include <QTreeWidget>
#include <QMenu>

class MT_MainWindow;

class MT_PopulationWidget : public MT_PopulationWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	MT_PopulationWidget(QMainWindow* parent=nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

private:
	int oldPopSize;
	QMenu *popContextMenu;
	QToolBar *popToolBar;
	QAction	*addIndAction;
	QAction	*delIndAction;
	QAction *impIndAction;
	QAction *expIndAction;
	QAction	*savePopAction;
	QAction	*loadPopAction;
	MT_Population *population;
	MT_GPManager  *gpManager;
	MT_MainWindow *mainWindow;
	void createActions();
	void createContextMenu();
	void createToolBar(QMainWindow *parent);
	QList<MT_PopListViewItem *> getSelectedItems();
	void exportAsPopulation(const QList<MT_PopListViewItem *> &selectedItems);
	void exportAsIndividuals(const QList<MT_PopListViewItem *> &selectedItems);

private slots:
	void slotRButtonClicked(const QPoint &pos);
	void slotCurrentChanged(QTreeWidgetItem *item);
	void slotSelectionChanged();
	void slotAddInd();
	void slotDelInd();
	void slotSavePop();
	void slotLoadPop();
	void slotExpInd();
	void slotImpInd();
	void slotNumChanged();

signals:
	void numChanged();
};

#endif