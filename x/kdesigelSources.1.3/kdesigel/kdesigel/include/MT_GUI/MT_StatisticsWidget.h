#ifndef MT_GUI_STATISTICSWIDGET
#define MT_GUI_STATISTICSWIDGET

#include "MT_GUI/MT_StatisticsWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"

#include <qtoolbar.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qaction.h>
#include <qtimer.h>

class MT_StatisticsWidget : public MT_StatisticsWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_StatisticsWidget(QMainWindow* parent=0, const char* name=0, WFlags fl=0);
	~MT_StatisticsWidget();

	void onShow(MT_GPManager *manager, subst_cache *subst);
	bool onHide(MT_GPManager *manager, subst_cache *subst);
	void evolutionRunning(bool running);

private:
	bool evolRunning;
	MT_Statistics *stat;
	MT_GPManager *mng;
	subst_cache *sigStat;

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