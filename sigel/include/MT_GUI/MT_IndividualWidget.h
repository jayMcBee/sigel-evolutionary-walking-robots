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
#ifndef MT_GUI_INDIVIDUAL
#define MT_GUI_INDIVIDUAL

#include "MT_GUI/MT_IndividualWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_Editor.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <QValidator>
#include <QMenu>
#include <QAction>
#include <QStringList>
#include <QTextStream>

class MT_IndividualsWidget : public MT_IndividualsWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

	friend class MT_AddConstantsWidget;

public:
	bool onHide( MT_GPManager *manager, MT_EstimationState *estimationState ) override;
	void onShow( MT_GPManager *manager, MT_EstimationState *estimationState ) override;

	MT_IndividualsWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

private slots:
	void slotCreateConstants();
	void slotImportConstants();
	void slotExportConstants();
	void slotChangeSlider( const QString &text );
	void slotChangeEdit( int value );
	void slotEditConst();
	void slotEditConst( QListWidgetItem *item );
	void slotChangeConstant( const QString &ntext );
	void slotRButtonClicked( const QPoint &pos );
	void slotSelectionChanged();
	void slotDelConst();
	void slotResetFocus();
	void slotNumConstChanged();

private:
	double minValue, maxValue;
	bool integer;
	int numToCreate;
	QStringList commandNames;

	MT_Randomizer *randomizer;

	QIntValidator *validator;
	QMenu *constContextMenu;
	QAction *delConstAction;
	QAction *editConstAction;
	MT_Editor *editor;

	void createWidgets();
	void createActions();
	void createContextMenu();
	void importConstantsFromFile( const QString &fileName );
	int readConstants( QTextStream &stream );
	void addConstants( QTextStream &stream, int count );

signals:
	void numConstChanged();
};

#endif