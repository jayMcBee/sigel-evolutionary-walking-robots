#include <QMenuBar>
#include <stdlib.h>

#include "MT_GUI/MT_MainWindow.h"

MT_MainWindow::MT_MainWindow(MT_Controller *controller, MT_GPManager *manager, subst_cache *substCache, QWidget * parent, const char * name, Qt::WindowFlags f )
 : QMainWindow( parent, f ), gpManager(manager), subst(substCache)
{
	if ( name )
		setObjectName( QString::fromUtf8( name ) );

	// Qt 2's WType_Modal did not merely name a window type -- it set
	// WState_Modal (qwidget.cpp:725) and show() called qt_enter_modal()
	// (qwidget.cpp:3365), so the MetaGP configuration window was APPLICATION
	// MODAL. Qt 6's Qt::Dialog is a window type only; windowModality defaults
	// to NonModal and nothing else sets it. The bit values happen to coincide
	// at 0x3, which is a coincidence and not an equivalence.
	// Qt::Dialog is Window|0x2, so `f & Qt::Dialog' is also true for a plain
	// Qt::Window, Qt::Tool, or anything else with bit 0 set. The window TYPE is
	// the masked value.
	if ( ( f & Qt::WindowType_Mask ) == Qt::Dialog )
		setWindowModality( Qt::ApplicationModal );
	evolRunning = false;
	boss = controller;
	evolTimer = new QTimer(this);

	// initialize variables
	actWidgetID = -1;

	QString pixPath = ::getenv("SIGEL_ROOT");
	pixPath += "/pixmaps/";

	resize(680, 595);
	setWindowTitle("SIGEL MetaGP");

	// add a statusbar and a tooltip group
	statusBar();

	// create tool- and menubar
	fileMenu = new QMenu(this);
	fileMenu->setObjectName("mtFileMenu");
	mainToolBar = new QToolBar(this);
	mainToolBar->setObjectName("mtFileToolBar");
	mainToolBar->setWindowTitle("MT File");
	// Qt 2's QToolBar(QMainWindow*, name) docked ITSELF -- the constructor
	// called parent->addToolBar(this, QString::null, QMainWindow::Top)
	// (qtoolbar.cpp:279). Qt 6's does not, so an undocked toolbar floats at
	// 0,0 over the central widget.
	addToolBar(Qt::TopToolBarArea, mainToolBar);

	// toolbar for evolution control
	evolCtrlToolbar = new QToolBar(this);
	evolCtrlToolbar->setObjectName("mtEvolutionToolBar");
	addToolBar(Qt::TopToolBarArea, evolCtrlToolbar);
	evolCtrlToolbar->setWindowTitle("MT Evolution Control");

	QIcon icon_mtStartEvolutionAction(QPixmap(pixPath+"mt_StartSmall.xpm"));
	icon_mtStartEvolutionAction.addPixmap(QPixmap(pixPath+"mt_StartLarge.xpm"));
	mtStartEvolutionAction = new QAction(icon_mtStartEvolutionAction, "Start", this);
	mtStartEvolutionAction->setToolTip("start evolution");
	mtStartEvolutionAction->setStatusTip("Starts the evolution without running SIGEL.");
	mtStartEvolutionAction->setEnabled(false);
	evolCtrlToolbar->addAction(mtStartEvolutionAction);
	QObject::connect(mtStartEvolutionAction, SIGNAL(triggered()), SLOT(slotStartEvolution()));

	QIcon icon_mtStopEvolutionAction(QPixmap(pixPath+"mt_StopSmall.xpm"));
	icon_mtStopEvolutionAction.addPixmap(QPixmap(pixPath+"mt_StopLarge.xpm"));
	mtStopEvolutionAction = new QAction(icon_mtStopEvolutionAction, "Stop", this);
	mtStopEvolutionAction->setToolTip("stop evolution");
	mtStopEvolutionAction->setStatusTip("Stops the evolution.");
	mtStopEvolutionAction->setEnabled(false);
	evolCtrlToolbar->addAction(mtStopEvolutionAction);
	QObject::connect(mtStopEvolutionAction, SIGNAL(triggered()), SLOT(slotStopEvolution()));

	mtEvolutionStatus = new QLabel(evolCtrlToolbar);
	mtEvolutionStatus->setObjectName("mtEvolStatus");
	mtEvolutionStatus->setText(" stopped ");
	// Qt 2's QToolBar::init() did boxLayout()->setAutoAdd(TRUE)
	// (qtoolbar.cpp:300): any child widget joined the toolbar's layout on
	// construction, in creation order. Qt 6 has no autoAdd, so a child parented
	// to a toolbar is an unmanaged overlay at 0,0 unless addWidget() is called.
	evolCtrlToolbar->addWidget(mtEvolutionStatus);

	evolCtrlToolbar->addSeparator();
	QIcon icon_mtAutoStopAction(QPixmap(pixPath+"mt_AutoStopSmall.xpm"));
	icon_mtAutoStopAction.addPixmap(QPixmap(pixPath+"mt_AutoStopLarge.xpm"));
	mtAutoStopAction = new QAction(icon_mtAutoStopAction, "manual/timed stop", this);
	mtAutoStopAction->setToolTip("manual/timed stop");
	mtAutoStopAction->setCheckable(true);
	mtAutoStopAction->setStatusTip("If on, evolution is stopped after the given time period is run down.");
	mtAutoStopAction->setChecked(false);
	evolCtrlToolbar->addAction(mtAutoStopAction);

	// max timed evolution duration is set to 24 days because the duration 
	// is to be calculated in milli seconds and stored in an integer
	// (assumed that an integer has 32bit)
	mtHour = new QSpinBox(evolCtrlToolbar);
	mtHour->setRange(0, 576);
	mtHour->setSingleStep(1);
	mtHour->setValue(0);
	mtTime1 = new QLabel(evolCtrlToolbar);
	mtTime1->setObjectName("mtTime1");
	mtTime1->setText("h : ");
	evolCtrlToolbar->addWidget(mtHour);
	evolCtrlToolbar->addWidget(mtTime1);
	mtMin = new QSpinBox(evolCtrlToolbar);
	mtMin->setRange(0, 864000);
	mtMin->setSingleStep(5);
	mtMin->setValue(0);
	mtTime2 = new QLabel(evolCtrlToolbar);
	mtTime2->setObjectName("mtTime2");
	mtTime2->setText("m ");
	evolCtrlToolbar->addWidget(mtMin);
	evolCtrlToolbar->addWidget(mtTime2);
	QObject::connect(mtAutoStopAction, SIGNAL(toggled(bool)), SLOT(slotAutoStop(bool)));
	QObject::connect(mtMin, SIGNAL(valueChanged(int)), SLOT(slotMinChanged(int)));
	
	// divide the window horizontally in to resizable parts
	splitter = new QSplitter(this);
	splitter->setObjectName("Splitter");
	splitter->setFrameStyle( QFrame::Box | QFrame::Sunken );
	splitter->setOpaqueResize();
	splitter->setContentsMargins(6, 6, 6, 6);

	setCentralWidget( splitter );		// make splitter the main widgets

	// set the selection tree into the left part of the window
	experimentWidget = new MT_ExperimentWidget(splitter);

	// put a stack of widgets into the right part of the window
	widgetStack = new QStackedWidget(splitter);
	widgetStack->setObjectName("WidgetStack");
	widgetStack->setContentsMargins(6, 6, 6, 6);
	widgetStack->setFrameStyle( QFrame::Box | QFrame::Sunken );

	widgets[0] = estimationWidget = new MT_EstimationWidget(this, "MTEstimationWidget", Qt::WindowFlags());
	widgetStack->insertWidget(0, estimationWidget);

	widgets[1] = individualsWidget = new MT_IndividualsWidget(this, "MTIndividualsWidget", Qt::WindowFlags());
	widgetStack->insertWidget(1, individualsWidget);

	widgets[2] = populationWidget = new MT_PopulationWidget(this, "MTPopulationWidget", Qt::WindowFlags());
	widgetStack->insertWidget(2, populationWidget);

	widgets[3] = searchWidget = new MT_SearchWidget(this, "MTSearchWidget", Qt::WindowFlags());
	widgetStack->insertWidget(3, searchWidget);

	widgets[4] = selectionWidget = new MT_SelectionWidget(this, "MTSelectionWidget", Qt::WindowFlags());
	widgetStack->insertWidget(4, selectionWidget);

	widgets[5] = statisticsWidget = new MT_StatisticsWidget(this, "MTStatisticsWidget", Qt::WindowFlags());
	widgetStack->insertWidget(5, statisticsWidget);

	// create the actions for the menu and the toolbar
	// - restore default settings
	QIcon icon_mtDefaultAction(QPixmap(pixPath+"newExperimentSmall.xpm"));
	icon_mtDefaultAction.addPixmap(QPixmap(pixPath+"newExperimentLarge.xpm"));
	mtDefaultAction = new QAction(icon_mtDefaultAction, "&Default", this);
	mtDefaultAction->setToolTip("Default");
	mtDefaultAction->setShortcut(Qt::CTRL | Qt::Key_D);
	mtDefaultAction->setStatusTip("Restores the default settings.");
	fileMenu->addAction(mtDefaultAction);
	mainToolBar->addAction(mtDefaultAction);
	fileMenu->addSeparator();
	mainToolBar->addSeparator();

	// - load settings
	QIcon icon_mtLoadAction(QPixmap(pixPath+"openExperimentSmall.xpm"));
	icon_mtLoadAction.addPixmap(QPixmap(pixPath+"openExperimentLarge.xpm"));
	mtLoadAction = new QAction(icon_mtLoadAction, "&Open", this);
	mtLoadAction->setToolTip("Load");
	mtLoadAction->setShortcut(Qt::CTRL | Qt::Key_O);
	mtLoadAction->setStatusTip("Load settings from a file.");
	fileMenu->addAction(mtLoadAction);
	mainToolBar->addAction(mtLoadAction);

	// - save settings
	QIcon icon_mtSaveAction(QPixmap(pixPath+"saveExperimentSmall.xpm"));
	icon_mtSaveAction.addPixmap(QPixmap(pixPath+"saveExperimentLarge.xpm"));
	mtSaveAction = new QAction(icon_mtSaveAction, "&Save", this);
	mtSaveAction->setToolTip("Save");
	mtSaveAction->setShortcut(Qt::CTRL | Qt::Key_S);
	mtSaveAction->setStatusTip("Saves the current settings to a file.");
	fileMenu->addAction(mtSaveAction);
	mainToolBar->addAction(mtSaveAction);
	fileMenu->addSeparator();
	mainToolBar->addSeparator();

	// - close window
	QIcon icon_mtExitAction(QPixmap(pixPath+"quitApplicationSmall.xpm"));
	icon_mtExitAction.addPixmap(QPixmap(pixPath+"quitApplicationLarge.xpm"));
	mtExitAction = new QAction(icon_mtExitAction, "E&xit", this);
	mtExitAction->setToolTip("Exit");
	mtExitAction->setShortcut(Qt::ALT | Qt::Key_F4);
	mtExitAction->setStatusTip("Applies changes and closes the configuration window.");
	fileMenu->addAction(mtExitAction);
	mainToolBar->addAction(mtExitAction);
	QObject::connect(mtExitAction, SIGNAL( triggered() ), SLOT( close() ));

	// Qt 2: QMenuBar::insertItem(text, popup). Qt 6 titles the menu itself.
	fileMenu->setTitle("&File");
	menuBar()->addMenu(fileMenu);

	QObject::connect(experimentWidget, // Qt 2 QListView::selectionChanged(item) carried the newly selected item;
	// Qt 6 has no such overload, and currentItemChanged is the one that does.
						 SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*) ),	SLOT( slotRaiseWidget(QTreeWidgetItem*) ));

	
	raiseWidget(0);
}

MT_MainWindow::~MT_MainWindow()
{
	QObject::disconnect(experimentWidget, 0, 0, 0);
}

void MT_MainWindow::slotAutoStop(bool on)
{
	if(on){
		mtHour->setEnabled(false);
		mtMin->setEnabled(false);
		if(evolRunning){
			int time = (mtHour->value() * 60 + mtMin->value()) * 60000;
			// Qt 2: QTimer::start(msec, singleShot). Qt 6 splits the two.
			evolTimer->setSingleShot(true);
			evolTimer->start(time);
		}
	} else {
		mtHour->setEnabled(true);
		mtMin->setEnabled(true);
		evolTimer->stop();
	}
}

void MT_MainWindow::slotMinChanged(int nvalue)
{
	if(nvalue > 59){
		int overflow = nvalue / 60;
		int rest = nvalue % 60;
		mtHour->setValue(mtHour->value() + overflow);
		mtMin->setValue(rest);
	}
}

void MT_MainWindow::showEvent(QShowEvent *e)
{
	QObject::connect(gpManager, SIGNAL( metaEvolutionRunning(bool) ), SLOT( slotEvolutionStatus(bool) ));
}

void MT_MainWindow::closeEvent(QCloseEvent *e)
{
	int topID = (actWidgetID == -1) ? 0 : actWidgetID;	// get widget position of the widget on top of the widgetstack
	MT_WidgetBase *topWidget = widgets[topID];			// get the top widget

	if(topWidget->onHide(gpManager, subst)){		// can we close the window savely ?
		QObject::disconnect(gpManager, SIGNAL( metaEvolutionRunning(bool) ), this, SLOT( slotEvolutionStatus(bool) ));

		QObject::disconnect(mtStartEvolutionAction, SIGNAL( triggered() ), (QObject*)boss, SLOT( startSingleEvolution() ));
		QObject::disconnect(mtStopEvolutionAction, SIGNAL( triggered() ), (QObject*)boss, SLOT( stopEvolution() ));
		QObject::disconnect(mtDefaultAction, SIGNAL( triggered() ), (QObject*)boss, SLOT(slotLoadDefault() ) );
		QObject::disconnect(mtLoadAction, SIGNAL( triggered() ), (QObject*)boss, SLOT( slotLoadSetup() ) );
		QObject::disconnect(mtSaveAction, SIGNAL( triggered() ), (QObject*)boss, SLOT( slotSaveSetup() ));
		QObject::disconnect(evolTimer, SIGNAL( timeout() ), (QObject*)boss, SLOT( stopEvolution() ));

		e->accept();	// yeah, close it
	} else
		e->ignore();	// sorry, keep it open
}

void MT_MainWindow::slotRaiseWidget(QTreeWidgetItem *item)
{
	// Same shape as MT_PopulationWidget::slotCurrentChanged: Qt 6's
	// currentItemChanged carries a null current where Qt 2's currentChanged
	// did not, e.g. after setCurrentItem(nullptr).
	if(!item) return;

	int pos = static_cast<MT_ExperimentItem*>(item)->getPos();
	MT_WidgetBase *nextWidget = widgets[pos];
	MT_WidgetBase *actWidget  = 0;

	if(!evolRunning)
		mtStartEvolutionAction->setEnabled( gpManager->separateEvolutionAllowed() );

	if(nextWidget){
		if(actWidgetID != -1){
			actWidget = widgets[actWidgetID];
			if(actWidget->onHide(gpManager, subst)){
				nextWidget->onShow(gpManager, subst);
				widgetStack->setCurrentIndex(pos);
				experimentWidget->slotCurrentChanged(item);
				actWidgetID = pos;
			} else {
				experimentWidget->lastSelected();
			}
		} else {
			nextWidget->onShow(gpManager, subst);
			widgetStack->setCurrentIndex(pos);
			actWidgetID = pos;
		}
	}
}

void MT_MainWindow::raiseWidget(int pos)
{
	MT_WidgetBase *nextWidget = widgets[pos];
	MT_WidgetBase *actWidget  = 0;

	if(!evolRunning)
		mtStartEvolutionAction->setEnabled( gpManager->separateEvolutionAllowed() );

	if(nextWidget){
		if(actWidgetID != -1){
			actWidget = widgets[actWidgetID];
			if(actWidget->onHide(gpManager, subst)){
				nextWidget->onShow(gpManager, subst);
				widgetStack->setCurrentIndex(pos);
				actWidgetID = pos;
			} else {
				experimentWidget->lastSelected();
			}
		} else {
			nextWidget->onShow(gpManager, subst);
			widgetStack->setCurrentIndex(pos);
			actWidgetID = pos;
		}
	}
}

MT_GPManager* MT_MainWindow::getManager()
{
	return gpManager;
}

void MT_MainWindow::enforceUpdate(bool GUI)
{
	MT_WidgetBase *actWidget  = 0;

	if(actWidgetID != -1){
		actWidget = widgets[actWidgetID];
		if(GUI){
			actWidget->onShow(gpManager, subst);
		} else {
			actWidget->onHide(gpManager, subst);
		}
	}
}

int MT_MainWindow::calculateTournSize(int pSize, int oSize, int oTSize)
{
	return selectionWidget->calculateTournSize(pSize, oSize, oTSize);
}

/***
 * evolution control methods
 ***/
void MT_MainWindow::slotEvolutionStatus(bool running)
{
	if(running){
		evolRunning = true;
		mtStartEvolutionAction->setEnabled(false);
		if(gpManager->separateEvolutionAllowed())
			mtStopEvolutionAction->setEnabled(true);
		mtEvolutionStatus->setText(" running ");
		if(mtAutoStopAction->isChecked()){
			int time = (mtHour->value() * 60 + mtMin->value()) * 60000;
			evolTimer->setSingleShot(true);
		evolTimer->start(time);
		}
		mtSaveAction->setEnabled(false);
		mtLoadAction->setEnabled(false);
		mtDefaultAction->setEnabled(false);
		searchWidget->evolutionRunning(true);
		selectionWidget->evolutionRunning(true);
		estimationWidget->evolutionRunning(true);
		populationWidget->evolutionRunning(true);
		individualsWidget->evolutionRunning(true);
		statisticsWidget->evolutionRunning(true);
	} else {
		evolRunning = false;
		if(gpManager->separateEvolutionAllowed())
			mtStartEvolutionAction->setEnabled(true);
		mtStopEvolutionAction->setEnabled(false);
		mtEvolutionStatus->setText(" stopped ");
		evolTimer->stop();
		mtSaveAction->setEnabled(true);
		mtLoadAction->setEnabled(true);
		mtDefaultAction->setEnabled(true);
		searchWidget->evolutionRunning(false);
		selectionWidget->evolutionRunning(false);
		estimationWidget->evolutionRunning(false);
		populationWidget->evolutionRunning(false);
		individualsWidget->evolutionRunning(false);
		statisticsWidget->evolutionRunning(false);
	}
}

void MT_MainWindow::slotStartEvolution()
{
	mtStartEvolutionAction->setEnabled(false);
	mtStopEvolutionAction->setEnabled(true);
	raiseWidget(5);
}

void MT_MainWindow::slotStopEvolution()
{
	mtStopEvolutionAction->setEnabled(false);
	if(!evolRunning)
		mtStartEvolutionAction->setEnabled(true);
	//boss->stopEvolution();
}
