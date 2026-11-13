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
#ifndef MT_GUI_SELECTIONWIDGET
#define MT_GUI_SELECTIONWIDGET

#include "MT_GUI/MT_SelectionWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"

#include <QMap>
#include <QString>

class MT_SelectionWidget : public MT_SelectionWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_SelectionWidget(QWidget* parent=nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState) override;

	int calculateTournamentSize(int pSize, int oSize, int oTSize);

private:
	int parentSize;
	int lastParentSize;
	QMap<int, int>	tournamentSizeMap;
	void updateTSizeList(int pSize, int oSize);

private slots:
	void slotOverProdChanged(int nvalue);
	void slotTournamentSizeChanged(const QString &text);
};

#endif