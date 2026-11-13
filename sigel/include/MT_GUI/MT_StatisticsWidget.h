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
#ifndef MT_GUI_STATISTICSWIDGET
#define MT_GUI_STATISTICSWIDGET

#include <QMainWindow>
#include "MT_GUI/MT_StatisticsWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"

#include <QToolBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QAction>
#include <QTimer>

class MT_StatisticsWidget : public MT_StatisticsWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_StatisticsWidget(QMainWindow* parent=nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	void evolutionRunning(bool running);

private:
	bool evolRunning;
	MT_Statistics *stat;
	MT_GPManager *mng;
	MT_EstimationState *estimationState;

	// toolbar
	QToolBar *statToolbar;
	QAction *updateAction;
	QLabel *intervalLabel;
	QSpinBox *intervalSpinBox;
	QCheckBox *autoUpdateCheckBox;
	QTimer *updateTimer;

	int soFarSigIdx, soFarMetIdx;
	int totalSim, totalEst;
	double totalMaxFit, totalAvgFit, totalVarFit;

	void updateGUI(int mtGen, int sigGen);
	void plotEstimation(QString fileName);
	void plotFitness(QString fileName);
	void plotSearchEffects(QString fileName);
	static int safePercentage(double part, int total);

private slots:
	void slotSigEdit(int nval);
	void slotSigSlider(int nval);
	void slotMtEdit(int nval);
	void slotMtSlider(int nval);
	void slotUpdateGUI();
	void slotAutoUpdateChanged(bool on);

	void slotEstGButton();
	void slotFitnessGButton();
	void slotSearchEffectsGButton();
	void slotEstDButton();
	void slotFitnessDButton();
	void slotSearchEffectsDButton();
	void slotEstPSExport();
	void slotFitnessPSExport();
	void slotSearchEffectsPSExport();
};

#endif