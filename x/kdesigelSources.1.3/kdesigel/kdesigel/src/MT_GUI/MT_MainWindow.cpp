#include <qmenubar.h>
#include <stdlib.h>

#include "MT_GUI/MT_MainWindow.h"

MT_MainWindow::MT_MainWindow(MT_Controller *controller, MT_GPManager *manager, subst_cache *substCache, QWidget * parent, const char * name, WFlags f )
 : QMainWindow( parent, name, f ), gpManager(manager), subst(substCache)
{
	evolRunning = false;
	boss = controller;
	evolTimer = new QTimer(this);

	// initialize variables
	actWidgetID = -1;

	QString pixPath = ::getenv("SIGEL_ROOT");
	pixPath += "/pixmaps/";

	resize(680, 595);
	setCaption("SIGEL MetaGP");

	// add a statusbar and a tooltip group
	statusBar();

	// create tool- and menubar
	fileMenu = new QPopupMenu(this, "mtFileMenu");
	mainToolBar = new QToolBar(this, "mtFileToolBar");
	mainToolBar->setLabel("MT File");

	// toolbar for evolution control
	evolCtrlToolbar = new QToolBar(this, "mtEvolutionToolBar");
	evolCtrlToolbar->setLabel("MT Evolution Control");

	mtStartEvolutionAction = new QAction("start evolution",
							QIconSet( QPixmap(pixPath+"mt_StartSmall.xpm"), QPixmap(pixPath+"mt_StartLarge.xpm") ),
							"Start",
							0, this);
	mtStartEvolutionAction->setStatusTip("Starts the evolution without running SIGEL.");
	mtStartEvolutionAction->setEnabled(false);
	mtStartEvolutionAction->addTo(evolCtrlToolbar);
	QObject::connect(mtStartEvolutionAction, SIGNAL(activated()), SLOT(slotStartEvolution()));

	mtStopEvolutionAction = new QAction("stop evolution",
							QIconSet( QPixmap(pixPath+"mt_StopSmall.xpm"), QPixmap(pixPath+"mt_StopLarge.xpm") ),
							"Stop",
							0, this);
	mtStopEvolutionAction->setStatusTip("Stops the evolution.");
	mtStopEvolutionAction->setEnabled(false);
	mtStopEvolutionAction->addTo(evolCtrlToolbar);
	QObject::connect(mtStopEvolutionAction, SIGNAL(activated()), SLOT(slotStopEvolution()));

	mtEvolutionStatus = new QLabel(evolCtrlToolbar, "mtEvolStatus");
	mtEvolutionStatus->setText(" stopped ");

	evolCtrlToolbar->addSeparator();
	mtAutoStopAction = new QAction("manual/timed stop",
							QIconSet( QPixmap(pixPath+"mt_AutoStopSmall.xpm"), QPixmap(pixPath+"mt_AutoStopLarge.xpm") ),
							"manual/timed stop",
							0, this, 0, true);
	mtAutoStopAction->setStatusTip("If on, evolution is stopped after the given time period is run down.");
	mtAutoStopAction->setOn(false);
	mtAutoStopAction->addTo(evolCtrlToolbar);

	// max timed evolution duration is set to 24 days because the duration 
	// is to be calculated in milli seconds and stored in an integer
	// (assumed that an integer has 32bit)
	mtHour = new QSpinBox(0, 576, 1, evolCtrlToolbar);
	mtHour->setValue(0);
	mtTime1 = new QLabel(evolCtrlToolbar, "mtTime1");
	mtTime1->setText("h : ");
	mtMin = new QSpinBox(0, 864000, 5, evolCtrlToolbar);
	mtMin->setValue(0);
	mtTime2 = new QLabel(evolCtrlToolbar, "mtTime2");
	mtTime2->setText("m ");
	QObject::connect(mtAutoStopAction, SIGNAL(toggled(bool)), SLOT(slotAutoStop(bool)));
	QObject::connect(mtMin, SIGNAL(valueChanged(int)), SLOT(slotMinChanged(int)));
	
	// divide the window horizontally in to resizable parts
	splitter = new QSplitter( this, "Splitter" );
	splitter->setFrameStyle( QFrame::Box | QFrame::Sunken );
	splitter->setOpaqueResize();
	splitter->setMargin( 6 );

	setCentralWidget( splitter );		// make splitter the main widgets

	// set the selection tree into the left part of the window
	experimentWidget = new MT_ExperimentWidget(splitter);

	// put a stack of widgets into the right part of the window
	widgetStack = new QWidgetStack( splitter, "WidgetStack" );
	widgetStack->setMargin( 6 );
	widgetStack->setFrameStyle( QFrame::Box | QFrame::Sunken );

	widgets[0] = estimationWidget = new MT_EstimationWidget(this, "MTEstimationWidget", 0);
	widgetStack->addWidget(estimationWidget, 0);

	widgets[1] = individualsWidget = new MT_IndividualsWidget(this, "MTIndividualsWidget", 0);
	widgetStack->addWidget(individualsWidget, 1);

	widgets[2] = populationWidget = new MT_PopulationWidget(this, "MTPopulationWidget", 0);
	widgetStack->addWidget(populationWidget, 2);

	widgets[3] = searchWidget = new MT_SearchWidget(this, "MTSearchWidget", 0);
	widgetStack->addWidget(searchWidget, 3);

	widgets[4] = selectionWidget = new MT_SelectionWidget(this, "MTSelectionWidget", 0);
	widgetStack->addWidget(selectionWidget, 4);

	widgets[5] = statisticsWidget = new MT_StatisticsWidget(this, "MTStatisticsWidget", 0);
	widgetStack->addWidget(statisticsWidget, 5);

	// create the actions for the menu and the toolbar
	// - restore default settings
	mtDefaultAction = new QAction("Default",
							QIconSet( QPixmap(pixPath+"newExperimentSmall.xpm"), QPixmap(pixPath+"newExperimentLarge.xpm") ),
							"&Default",
							CTRL+Key_D, this);
	mtDefaultAction->setStatusTip("Restores the default settings.");
	mtDefaultAction->addTo(fileMenu);
	mtDefaultAction->addTo(mainToolBar);
	fileMenu->insertSeparator();
	mainToolBar->addSeparator();

	// - load settings
	mtLoadAction = new QAction("Load",
							QIconSet( QPixmap(pixPath+"openExperimentSmall.xpm"), QPixmap(pixPath+"openExperimentLarge.xpm") ),
							"&Open",
							CTRL+Key_O, this);
	mtLoadAction->setStatusTip("Load settings from a file.");
	mtLoadAction->addTo(fileMenu);
	mtLoadAction->addTo(mainToolBar);

	// - save settings
	mtSaveAction = new QAction("Save",
							QIconSet( QPixmap(pixPath+"saveExperimentSmall.xpm"), QPixmap(pixPath+"saveExperimentLarge.xpm") ),
							"&Save",
							CTRL+Key_S, this);
	mtSaveAction->setStatusTip("Saves the current settings to a file.");
	mtSaveAction->addTo(fileMenu);
	mtSaveAction->addTo(mainToolBar);
	fileMenu->insertSeparator();
	mainToolBar->addSeparator();

	// - close window
	mtExitAction = new QAction("Exit",
							QIconSet( QPixmap(pixPath+"quitApplicationSmall.xpm"), QPixmap(pixPath+"quitApplicationLarge.xpm") ),
							"E&xit",
							ALT+Key_F4, this);
	mtExitAction->setStatusTip("Applies changes and closes the configuration window.");
	mtExitAction->addTo(fileMenu);
	mtExitAction->addTo(mainToolBar);
	QObject::connect(mtExitAction, SIGNAL( activated() ), SLOT( close() ));

	menuBar()->insertItem("&File", fileMenu);

	QObject::connect(experimentWidget, SIGNAL( selectionChanged(QListViewItem*) ),	SLOT( slotRaiseWidget(QListViewItem*) ));

	
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
			evolTimer->start(time, true);
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

		QObject::disconnect(mtStartEvolutionAction, SIGNAL( activated() ), (QObject*)boss, SLOT( startSingleEvolution() ));
		QObject::disconnect(mtStopEvolutionAction, SIGNAL( activated() ), (QObject*)boss, SLOT( stopEvolution() ));
		QObject::disconnect(mtDefaultAction, SIGNAL( activated() ), (QObject*)boss, SLOT(slotLoadDefault() ) );
		QObject::disconnect(mtLoadAction, SIGNAL( activated() ), (QObject*)boss, SLOT( slotLoadSetup() ) );
		QObject::disconnect(mtSaveAction, SIGNAL( activated() ), (QObject*)boss, SLOT( slotSaveSetup() ));
		QObject::disconnect(evolTimer, SIGNAL( timeout() ), (QObject*)boss, SLOT( stopEvolution() ));

		e->accept();	// yeah, close it
	} else
		e->ignore();	// sorry, keep it open
}

void MT_MainWindow::slotRaiseWidget(QListViewItem *item)
{
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
				widgetStack->raiseWidget(pos);
				experimentWidget->slotCurrentChanged(item);
				actWidgetID = pos;
			} else {
				experimentWidget->lastSelected();
			}
		} else {
			nextWidget->onShow(gpManager, subst);
			widgetStack->raiseWidget(pos);
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
				widgetStack->raiseWidget(pos);
				actWidgetID = pos;
			} else {
				experimentWidget->lastSelected();
			}
		} else {
			nextWidget->onShow(gpManager, subst);
			widgetStack->raiseWidget(pos);
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
		if(mtAutoStopAction->isOn()){
			int time = (mtHour->value() * 60 + mtMin->value()) * 60000;
			evolTimer->start(time, true);
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
