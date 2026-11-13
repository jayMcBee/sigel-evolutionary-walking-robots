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
#ifndef MT_GUI_ESTIMATIONWIDGET
#define MT_GUI_ESTIMATIONWIDGET

#include "MT_GUI/MT_EstimationWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/DoubleSpinBox.h"
#include "MT_Control/MT_SubstituteKind.h"

class MT_EstimationWidget : public MT_EstimationWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_EstimationWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

	bool onHide( MT_GPManager *manager, MT_EstimationState *estimationState ) override;
	void onShow( MT_GPManager *manager, MT_EstimationState *estimationState ) override;

public slots:
	void slotStrategyChanged( int index );

private:
	MT_SubstituteKind usedSys;
	DISpinBox *toleranceSpinBox;
	int lastIndex;
	double tol0, tol1, tol2, tol3, tol4;
};

#endif