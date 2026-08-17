#include <qmainwindow.h>
#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qlabel.h>
#include <qtimer.h>

#include "MT_GUI/MT_ExperimentWidget.h"
#include "MT_GUI/MT_EstimationWidget.h"
#include "MT_GUI/MT_IndividualWidget.h"
#include "MT_GUI/MT_PopulationWidget.h"
#include "MT_GUI/MT_StatisticsWidget.h"
#include "MT_GUI/MT_SelectionWidget.h"
#include "MT_GUI/MT_SearchWidget.h"

#include "MT_GPSystem/MT_Randomizer.h"
#include "MT_GPSystem/MT_GPManager.h"
#include "MT_Control/MT_Substitute.h"


class MT_Controller;

class MT_MainWindow : public QMainWindow
{
	Q_OBJECT

	friend class MT_Controller;

public:
	MT_MainWindow(MT_Controller *controller, MT_GPManager *manager, subst_cache *substCache, QWidget * parent = 0, const char * name = 0, WFlags f = WType_TopLevel | WType_Modal );
	~MT_MainWindow();

	MT_GPManager* getManager();
	void enforceUpdate(bool GUI);
	int calculateTournSize(int pSize, int oSize, int oTSize);

public slots:
	void slotRaiseWidget(QListViewItem *item);
	void slotEvolutionStatus(bool running);

private slots:
	void slotStartEvolution();
	void slotStopEvolution();
	void slotAutoStop(bool on);
	void slotMinChanged(int nvalue);

private:
	MT_WidgetBase* widgets[6];
	int actWidgetID;
	QSplitter	*splitter;
	QWidgetStack	*widgetStack;
	MT_SearchWidget		*searchWidget;
	MT_SelectionWidget	*selectionWidget;
	MT_ExperimentWidget	*experimentWidget;
	MT_EstimationWidget	*estimationWidget;
	MT_PopulationWidget *populationWidget;
	MT_StatisticsWidget	*statisticsWidget;
	MT_IndividualsWidget *individualsWidget;
	MT_Controller *boss;

	QToolBar	*mainToolBar;
	QPopupMenu	*fileMenu;
	QAction	*mtExitAction;
	QAction *mtSaveAction;
	QAction *mtLoadAction;
	QAction *mtDefaultAction;

	bool evolRunning;
	QToolBar *evolCtrlToolbar;
	QAction *mtStartEvolutionAction;
	QAction *mtStopEvolutionAction;
	QLabel *mtEvolutionStatus;
	QAction *mtAutoStopAction;
	QSpinBox *mtHour;
	QSpinBox *mtMin;
	QLabel *mtTime1, *mtTime2;
	QTimer *evolTimer;

	MT_Randomizer *random;
	MT_GPManager *gpManager;
	subst_cache *subst;

	void raiseWidget(int pos);

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void showEvent(QShowEvent *e);
};