#include "MT_GUI/MT_PopulationWidget.h"

// Qt 2's QListViewItem::nextSibling() has no Qt 6 equivalent. Every item in
// this view is top level, so the next sibling is the next top-level index.
static MT_PopListViewItem *nextSiblingOf(QTreeWidget *tree, QTreeWidgetItem *item)
{
	if(!item) return 0;
	int i = tree->indexOfTopLevelItem(item);
	if(i < 0 || i + 1 >= tree->topLevelItemCount()) return 0;
	return (MT_PopListViewItem *)tree->topLevelItem(i + 1);
}
#include "MT_GUI/MT_AddIndividualsWidget.h"
#include "MT_GUI/MT_PopListViewItem.h"
#include "MT_GUI/MT_MainWindow.h"

#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Programline.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <QSpinBox>
#include <QProgressDialog>
#include <QFileDialog>
#include <QStringList>
#include <QTextEdit>
#include <QLCDNumber>
#include <QMessageBox>


MT_PopulationWidget::MT_PopulationWidget(QMainWindow* parent, const char* name, Qt::WindowFlags fl)
	: MT_PopulationWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	oldPopSize = -1;
	boss = (MT_MainWindow *) parent;

	QString pixPath = ::getenv("SIGEL_ROOT");
	pixPath += "/pixmaps/";

	// create the toolbar
	// Qt 2's QToolBar(QMainWindow*, name) docked itself (qtoolbar.cpp:279).
	popToolBar = new QToolBar(parent);
	// Qt 2 guarded this: `if ( parent ) parent->addToolBar(...)'
	// (qtoolbar.cpp:278-279), warning rather than crashing otherwise.
	if(parent)
		parent->addToolBar(Qt::TopToolBarArea, popToolBar);
	popToolBar->setObjectName("mtPopToolBar");
	popToolBar->hide();
	popToolBar->setWindowTitle("MT Population");

	// create context menu
	popContextMenu = new QMenu(this);
	popContextMenu->setObjectName("mtPopContextMenu");

	// create the actions and insert them
	QIcon icon_addIndAction(QPixmap(pixPath+"addIndividualsSmall.xpm"));
	icon_addIndAction.addPixmap(QPixmap(pixPath+"addIndividualsLarge.xpm"));
	addIndAction = new QAction(icon_addIndAction, "&Add", parentWindow);
	addIndAction->setToolTip("Add Individual");
	addIndAction->setStatusTip("Adds a randomly created individual to the population.");
	popToolBar->addAction(addIndAction);
	popContextMenu->addAction(addIndAction);

	QIcon icon_delIndAction(QPixmap(pixPath+"deleteIndividualsSmall.xpm"));
	icon_delIndAction.addPixmap(QPixmap(pixPath+"deleteIndividualsLarge.xpm"));
	delIndAction = new QAction(icon_delIndAction, "&Delete", parentWindow);
	delIndAction->setToolTip("Delete Individual");
	delIndAction->setEnabled(false);
	delIndAction->setStatusTip("Deletes the selected individual from the population.");
	popToolBar->addAction(delIndAction);
	popContextMenu->addAction(delIndAction);

	QIcon icon_impIndAction(QPixmap(pixPath+"mt_ImpIndSmall.xpm"));
	icon_impIndAction.addPixmap(QPixmap(pixPath+"/mt_ImpIndSmall.xpm"));
	impIndAction = new QAction(icon_impIndAction, "&Import", parentWindow);
	impIndAction->setToolTip("Import Individual");
	impIndAction->setStatusTip("Imports one or more individuals from a file.");
	popToolBar->addAction(impIndAction);
	popContextMenu->addAction(impIndAction);

	QIcon icon_expIndAction(QPixmap(pixPath+"mt_ExpIndSmall.xpm"));
	icon_expIndAction.addPixmap(QPixmap(pixPath+"mt_ExpIndSmall.xpm"));
	expIndAction = new QAction(icon_expIndAction, "&Export", parentWindow);
	expIndAction->setToolTip("Export Individual");
	expIndAction->setStatusTip("Writes the selected Individuals to a file.");
	popToolBar->addAction(expIndAction);
	popContextMenu->addAction(expIndAction);

	QIcon icon_loadPopAction(QPixmap(pixPath+"openExperimentSmall.xpm"));
	icon_loadPopAction.addPixmap(QPixmap(pixPath+"openExperimentLarge.xpm"));
	loadPopAction = new QAction(icon_loadPopAction, "&Load Population", parentWindow);
	loadPopAction->setToolTip("Load Population");
	loadPopAction->setStatusTip("Loads a population from a file and adds it to the current population.");
	popToolBar->addAction(loadPopAction);

	QIcon icon_savePopAction(QPixmap(pixPath+"saveExperimentSmall.xpm"));
	icon_savePopAction.addPixmap(QPixmap(pixPath+"saveExperimentLarge.xpm"));
	savePopAction = new QAction(icon_savePopAction, "&Save Population", parentWindow);
	savePopAction->setToolTip("Save Population");
	savePopAction->setStatusTip("Save the complete population to a file.");
	popToolBar->addAction(savePopAction);

	// establish connections
	individualListView->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect((const QObject*) individualListView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotRButtonClicked(const QPoint&)));
	QObject::connect((const QObject*) individualListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotCurrentChanged(QTreeWidgetItem*)));
	QObject::connect((const QObject*) individualListView, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()));
	QObject::connect(addIndAction, SIGNAL(triggered()), SLOT(slotAddInd()));
	QObject::connect(delIndAction, SIGNAL(triggered()), SLOT(slotDelInd()));
	QObject::connect(impIndAction, SIGNAL(triggered()), SLOT(slotImpInd()));
	QObject::connect(expIndAction, SIGNAL(triggered()), SLOT(slotExpInd()));
	QObject::connect(loadPopAction, SIGNAL(triggered()), SLOT(slotLoadPop()));
	QObject::connect(savePopAction, SIGNAL(triggered()), SLOT(slotSavePop()));
	QObject::connect(this, SIGNAL(numChanged()), SLOT(slotNumChanged()));
};

MT_PopulationWidget::~MT_PopulationWidget()
{};

/***
 * enables/disables widgets during evolution
 ***/
void MT_PopulationWidget::evolutionRunning(bool running)
{
	if(running){

	} else {

	}
}

/***
 * reads information from the gp-system and displays it
 ***/
void MT_PopulationWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	gpManager = manager;

	// clear the widget
	// Qt 2's QListView::clear() blocked signals for its whole body
	// (qlistview.cpp:2303-2304, 2341), so currentChanged NEVER fired with a
	// null item. Qt 6's clear() emits currentItemChanged(nullptr, prev), which
	// slotCurrentChanged dereferences. Blocking reproduces 1.3 exactly.
	{
		const bool wasBlocked = individualListView->blockSignals(true);
		individualListView->clear();
		individualListView->blockSignals(wasBlocked);
	}
	individualProgramView->setText("");
	individualCountLCD->display(0);

	// get the current population
	population = manager->getParent();
	if(!population){
		QMessageBox::critical(this, "Configure Meta-System", "Couldn't get current population.", "Ok");
		return;
	}
	oldPopSize = population->getSize();

	// list all individuals
	MT_Individual *actInd;
	for(int i=0; i<population->getSize(); i++){
		actInd = population->getIndividual(i);
		new MT_PopListViewItem(individualListView, actInd);
		emit numChanged();
	}

	popToolBar->show();
}

/***
 * checks contradictions and solves them
 * returns true if the widget can be savely closed/changed
 ***/
bool MT_PopulationWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	int popSize = individualListView->topLevelItemCount();

	if(oldPopSize != popSize){
		int offspringSize;
		int tournSize;
		int selMethod;
		int fitFunc;
		int tsetSize;
		int tDuration;

		manager->getSelektionValue(&offspringSize, &tournSize, 
			&selMethod, &fitFunc, 
			&tsetSize, &tDuration);

		// calculate the new offspring size
		int overProdFac = offspringSize / oldPopSize;
		offspringSize   = popSize * overProdFac;

		// offspring population size has changed so we have to recalculate a tournament size
		tournSize = boss->calculateTournSize(popSize, offspringSize, tournSize);

		// ... and finally set it
		manager->setSelektionValue(offspringSize, tournSize,
			selMethod, fitFunc,
			tsetSize, tDuration);

		manager->setPopAndTournamentSize(popSize, tournSize);
	}

	popToolBar->hide();
	return true;
}

/***
 * just updates the individual counter
 ***/
void MT_PopulationWidget::slotNumChanged()
{
	individualCountLCD->display( individualListView->topLevelItemCount());
}

/***
 * displays the context menu if a right click on a individual occurs
 ***/ 
void MT_PopulationWidget::slotRButtonClicked(const QPoint &pos)
{
	// customContextMenuRequested gives viewport coordinates, and a click on
	// blank space must clear the selection so Delete greys out.
	if(!individualListView->itemAt(pos))
		individualListView->clearSelection();
	popContextMenu->popup(individualListView->viewport()->mapToGlobal(pos));
}

/***
 * checks if an individual is selected
 * enables/disables the delete function
 ***/
void MT_PopulationWidget::slotSelectionChanged()
{
	QTreeWidgetItem *actItem = individualListView->currentItem();
	if(individualListView->topLevelItemCount() != 1 && actItem && actItem->isSelected())
	{
		delIndAction->setEnabled(true);
	} else {
		delIndAction->setEnabled(false);
	}
}

/***
 * displays the program if the currently selected individual
 * (if there is an individual selected)
 ***/
void MT_PopulationWidget::slotCurrentChanged(QTreeWidgetItem *item)
{
	// clean the display
	individualProgramView->clear();

	if(!item) return;

	// get the currently selected individual
	int pos = ((MT_PopListViewItem *) item)->getPos();
	MT_Individual *actInd = population->getIndividual(pos);

	// display the program
	for(int i=0; i<actInd->getProgram()->getLength(); i++){
		individualProgramView->append(actInd->printProgramLine(i));
	}
}

/***
 * add n randomly created individuals to the population
 ***/
void MT_PopulationWidget::slotAddInd()
{
	// opens a window in which the user can enter
	// the number of individuals to create
	// shows a progressbar
	MT_AddIndividualsWidgetBase numDialog(this, 0, true);
	int number = 0;

	if(QDialog::Accepted  == numDialog.exec()){

		// ok button pressed
		number = numDialog.spinboxNumber->value();

		QProgressDialog progress("Generating individuals", QString(), 0, number, this);
		progress.setWindowModality(Qt::ApplicationModal);

		MT_Individual *newInd;
		MT_Randomizer *rand = gpManager->getRandomizer();
		for(int i=0; i<number; i++){
			progress.setValue(i);
			int pos = population->createNewIndi(rand);
			newInd = population->getIndividual(pos);
			new MT_PopListViewItem(individualListView, newInd);
			emit numChanged();
		}
		progress.setValue(number);
	}

}

/***
 * removes all selected individuals from the population, resizes the
 * population and deletes the removed individuals
 ***/
void MT_PopulationWidget::slotDelInd()
{
	MT_PopListViewItem* actIndNew =0;
	
	MT_PopListViewItem *nextInd=0;
	MT_PopListViewItem *actInd = (MT_PopListViewItem*) individualListView->topLevelItem(0);

	int DelPos =0;
	int ActPos =0;

	// iterate over all items, check if they are selected
	// and delete the selected ones
	MT_Individual *actRInd =0;
	while(actInd){
		nextInd = nextSiblingOf(individualListView, actInd);	// get next individual in list
		if (( actInd->isSelected()) && (individualListView->topLevelItemCount() != 1) )
		{
			DelPos = actInd->getPos();
			actRInd = population->delIndividual(DelPos);
			delete actRInd;
			emit numChanged();
			
			actIndNew = (MT_PopListViewItem*) individualListView->topLevelItem(0);
			while(actIndNew !=0)		// walk over all IndisItem  // for every item with Pos > DelPos --> setPos(Pos-1)
			{
				ActPos = actIndNew->getPos();
				if (ActPos > DelPos)
					actIndNew->setPos(ActPos-1);
				
				actIndNew = nextSiblingOf(individualListView, actIndNew);	// get next individual in list
			}
			
			delete actInd;					// delete actual individual if selected
		}

		actInd = nextInd;
	}

}

/***
 * reads individuals from a file and appends them to the population
 ***/
void MT_PopulationWidget::slotImpInd()
{
	QStringList files( QFileDialog::getOpenFileNames( this, "Import Individuals", QString(), "Individuals(*.mind);;All Files(*)"));

	if(!files.isEmpty()){
	
		// iterate over all selected files
		QStringList::Iterator it = files.begin();
		uint i = 0;
		uint count = files.count();
		QProgressDialog progress("Importing individuals", QString(), 0, count, this);
		progress.setWindowModality(Qt::ApplicationModal);
		for( ; it != files.end(); ++it){
			QFile file(*it);
			progress.setValue(i++);
			if(file.open(QIODevice::ReadOnly)){		// file successfully opened
				QTextStream str(&file);

				//population->importPop(str);
				MT_Individual *newInd = new MT_Individual(str);
				population->addIndividual(newInd);
				new MT_PopListViewItem(individualListView, newInd);
				
				 emit numChanged();
				file.close();
			}
		}
		progress.setValue(count);
	}
}

/***
 * save the selected individual
 * if more than one is selected save them as a population or serial
 ***/
void MT_PopulationWidget::slotExpInd()
{
	QList<MT_PopListViewItem *> *list = getSelectedItems();

	bool saveAsPop = false;
	QString fileName;
	if(list->count() > 1){
		saveAsPop = !QMessageBox::information(this, "Save individuals", "There are more than one individual selected.\n"
			"Shall we save them as a population?", "Save as population", "Save separately");
	}
	if(saveAsPop){
		fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)");
		if(!fileName.isEmpty()){
			if(fileName.right(5) != ".mpop")
				fileName += ".mpop";
			QFile file( fileName );

			if(file.exists()){
				if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
					"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
					return;
			}

			if(file.open(QIODevice::WriteOnly)){
				QTextStream str(&file);
				
				MT_Population npop;
				npop.changePopSize(0);//list->count());
				// Qt 2's QPtrList carried an internal cursor: first() then
				// next() walked it. Qt 6 has no cursor; the index does the job.
				for(int i=0; i<list->count(); i++){
					MT_PopListViewItem *actItem = list->at(i);
					npop.addIndividual(population->getIndividual(actItem->getPos()));//, i);
				}

				npop.exportPop(str);
				npop.flush();
				

				file.close();
			} else {
				QMessageBox::critical(this, "Save population", "An error occured during saving the population.\nAborting operation.", "Ok");
			}
		}
	} else {
		fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Individual Files (*.mind);;All Files (*)");
		if(!fileName.isEmpty()){

			for(int i=0; i<list->count(); i++){
				QFile file( fileName.append("%1.mind").arg(i) );
				if(file.exists()){
					if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
						"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
						return;
				}
				if(file.open(QIODevice::WriteOnly)){
					QTextStream str(&file);
					// PRESERVED DEFECT: current() is the QPtrList cursor, which
					// this loop never advances -- append() left it on the LAST
					// selected item, so 1.3 writes that same individual into
					// every one of the N files. last() reproduces it exactly.
					population->getIndividual(list->last()->getPos())->writeToFileIndi(str);
					file.close();
				} else {
					QMessageBox::critical(this, "Save individual", "An error occured during saving the individual.\nAborting operation.", "Ok");
				}
			}

		}
	}

}

/***
 * load a saved population
 ***/
void MT_PopulationWidget::slotLoadPop()
{
	QString fileName( QFileDialog::getOpenFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)") );

	if(!fileName.isEmpty()){
		QFile file( fileName );

		if(file.open(QIODevice::ReadOnly)){
			QTextStream str(&file);

			if(1 == QMessageBox::warning(this, "Import population",
				"Shall the current population be deleted or shall we append\n"
				"the new individuals?", "append", "delete"))
			{
				// delete
				population->loadPop(str);
				onShow(gpManager, 0);	// update the GUI

			} else {
				// append
				population->importPop(str);
				/*
				MT_Population npop(str);
				MT_Individual *newInd = new MT_Individual(str);
				population->changePopSize(population->getSize()+npop.getSize());

				for(int i=0; i<npop.getSize(); i++)
					population->insertIndividual(npop.getIndividual(i));

				npop.flush();
				*/
				onShow(gpManager, 0);

			}
			file.close();
		}
	}
}

/***
 * export the current population to a file
 ***/
void MT_PopulationWidget::slotSavePop()
{
	QString fileName( QFileDialog::getSaveFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)") );

	if(!fileName.isEmpty()){
		if(fileName.right(5) != ".mpop")
			fileName += ".mpop";
		QFile file( fileName );

		if(file.exists()){
			if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
				"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
				return;
		}

		if(file.open(QIODevice::WriteOnly)){
			QTextStream str(&file);
			population->writeToFilePop(str);
			file.close();
		} else {
			QMessageBox::critical(this, "Save population", "An error occured during saving the population.\nAborting operation.", "Ok");
		}
	}
}

/***
 * collects all selected items
 ***/
QList<MT_PopListViewItem *> * MT_PopulationWidget::getSelectedItems()
{
	// Qt 2's QPtrList held pointers and this one explicitly did NOT own them
	// (setAutoDelete(false)); a Qt 6 QList of pointers never owns, so the flag
	// has no counterpart and is dropped rather than translated.
	QList<MT_PopListViewItem *> *lst = new QList<MT_PopListViewItem *>;
	QTreeWidgetItemIterator it(individualListView);
	for(; (*it); ++it){
		if((*it)->isSelected())
			lst->append((MT_PopListViewItem*)(*it));
	}
	return lst;
}