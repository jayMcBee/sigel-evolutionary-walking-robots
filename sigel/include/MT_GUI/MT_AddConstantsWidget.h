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
#ifndef MT_ADDCONSTANTSWIDGET
#define MT_ADDCONSTANTSWIDGET

#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_IndividualWidget.h"

#include <QValidator>

class MT_AddConstantsWidget : public MT_AddConstantsWidgetBase
{
	Q_OBJECT

public:
	MT_AddConstantsWidget(MT_IndividualsWidget *parent=nullptr, bool modal=true, Qt::WindowFlags fl = Qt::WindowFlags() );

private:
	// uic emits only the QGroupBox, so the button group is built here; without
	// it find(id) and clicked(int) have nothing to talk to.
	QButtonGroup *typeButtons;

	MT_IndividualsWidget *individualsWidget;
	enum type { intType, floatType } selectedType;
	QValidator	*minValidator;
	QValidator	*maxValidator;

public slots:
	void slotClicked(int id);
	void accept() override;
};

#endif