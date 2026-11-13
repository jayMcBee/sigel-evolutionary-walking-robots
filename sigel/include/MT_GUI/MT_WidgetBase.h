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
#ifndef MT_GUI_WIDGETBASE
#define MT_GUI_WIDGETBASE

#include <QWidget>

#include "MT_GPSystem/MT_GPManager.h"
#include "MT_Control/MT_EstimationState.h"
#include "MT_Control/MT_Substitute.h"

class MT_WidgetBase  
{
public:
	MT_WidgetBase(QWidget *parent);
	virtual ~MT_WidgetBase() = default;

	virtual bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState);
	virtual void onShow(MT_GPManager *manager, MT_EstimationState *estimationState);

protected:
	QWidget *parentWindow;
};

#endif
