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
#ifndef MT_GUI_SEARCHWIDGET
#define MT_GUI_SEARCHWIDGET

#include "MT_GUI/MT_SearchWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/DoubleSpinBox.h"

#include <QValidator>

class MT_SearchWidget : public MT_SearchWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_SearchWidget(QWidget* parent=nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState) override;

private slots:
	void slotXOverChanged(int nvalue);
	void slotMutChanged(int nvalue);
	void slotReproChanged(int nvalue);

	void slotXEditChanged(const QString &valText);
	void slotMEditChanged(const QString &valText);
	void slotREditChanged(const QString &valText);

	void slotpt1Changed(int nvalue);
	void slotpt2Changed(int nvalue);
	void slotpt3Changed(int nvalue);

private:
	QDoubleValidator *validator;
	DISpinBox *powerSpinBox;
	DISpinBox *lineProbSpinBox;

	void adjustProbs(int s1, int *p2, int *p3);
	void disconnectLEs();
	void reconnectLEs();
};

#endif