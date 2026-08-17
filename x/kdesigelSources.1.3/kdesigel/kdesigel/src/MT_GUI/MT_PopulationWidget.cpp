#include "MT_GUI/MT_PopulationWidget.h"
#include "MT_GUI/MT_AddIndividualsWidget.h"
#include "MT_GUI/MT_PopListViewItem.h"
#include "MT_GUI/MT_MainWindow.h"

#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Programline.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <qspinbox.h>
#include <qprogressdialog.h>
#include <qfiledialog.h>
#include <qstringlist.h>
#include <qmultilineedit.h>
#include <qlcdnumber.h>
#include <qmessagebox.h>


MT_PopulationWidget::MT_PopulationWidget(QMainWindow* parent, const char* name, WFlags fl)
	: MT_PopulationWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	oldPopSize = -1;
	boss = (MT_MainWindow *) parent;

	QString pixPath = ::getenv("SIGEL_ROOT");
	pixPath += "/pixmaps/";

	// create the toolbar
	popToolBar = new QToolBar(parent, "mtPopToolBar");
	popToolBar->hide();
	popToolBar->setLabel("MT Population");

	// create context menu
	popContextMenu = new QPopupMenu(this, "mtPopContextMenu");

	// create the actions and insert them
	addIndAction = new QAction("Add Individual",
		QIconSet(QPixmap(pixPath+"addIndividualsSmall.xpm"),QPixmap(pixPath+"addIndividualsLarge.xpm")),
		"&Add",
		0, parentWindow);
	addIndAction->setStatusTip("Adds a randomly created individual to the population.");
	addIndAction->addTo(popToolBar);
	addIndAction->addTo(popContextMenu);

	delIndAction = new QAction("Delete Individual",
		QIconSet(QPixmap(pixPath+"deleteIndividualsSmall.xpm"),QPixmap(pixPath+"deleteIndividualsLarge.xpm")),
		"&Delete",
		0, parentWindow);
	delIndAction->setEnabled(false);
	delIndAction->setStatusTip("Deletes the selected individual from the population.");
	delIndAction->addTo(popToolBar);
	delIndAction->addTo(popContextMenu);

	impIndAction = new QAction("Import Individual",
		QIconSet(QPixmap(pixPath+"mt_ImpIndSmall.xpm"),QPixmap(pixPath+"/mt_ImpIndSmall.xpm")),
		"&Import",
		0, parentWindow);
	impIndAction->setStatusTip("Imports one or more individuals from a file.");
	impIndAction->addTo(popToolBar);
	impIndAction->addTo(popContextMenu);

	expIndAction = new QAction("Export Individual",
		QIconSet(QPixmap(pixPath+"mt_ExpIndSmall.xpm"),QPixmap(pixPath+"mt_ExpIndSmall.xpm")),
		"&Export",
		0, parentWindow);
	expIndAction->setStatusTip("Writes the selected Individuals to a file.");
	expIndAction->addTo(popToolBar);
	expIndAction->addTo(popContextMenu);

	loadPopAction = new QAction("Load Population",
		QIconSet(QPixmap(pixPath+"openExperimentSmall.xpm"),QPixmap(pixPath+"openExperimentLarge.xpm")),
		"&Load Population",
		0, parentWindow);
	loadPopAction->setStatusTip("Loads a population from a file and adds it to the current population.");
	loadPopAction->addTo(popToolBar);

	savePopAction = new QAction("Save Population",
		QIconSet(QPixmap(pixPath+"saveExperimentSmall.xpm"),QPixmap(pixPath+"saveExperimentLarge.xpm")),
		"&Save Population",
		0, parentWindow);
	savePopAction->setStatusTip("Save the complete population to a file.");
	savePopAction->addTo(popToolBar);

	// establish connections
	QObject::connect((const QObject*) individualListView, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint&, int)), SLOT(slotRButtonClicked(QListViewItem*, const QPoint&) ));
	QObject::connect((const QObject*) individualListView, SIGNAL(currentChanged(QListViewItem*)), SLOT(slotCurrentChanged(QListViewItem*)));
	QObject::connect((const QObject*) individualListView, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	QObject::connect(addIndAction, SIGNAL(activated()), SLOT(slotAddInd()));
	QObject::connect(delIndAction, SIGNAL(activated()), SLOT(slotDelInd()));
	QObject::connect(impIndAction, SIGNAL(activated()), SLOT(slotImpInd()));
	QObject::connect(expIndAction, SIGNAL(activated()), SLOT(slotExpInd()));
	QObject::connect(loadPopAction, SIGNAL(activated()), SLOT(slotLoadPop()));
	QObject::connect(savePopAction, SIGNAL(activated()), SLOT(slotSavePop()));
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
	individualListView->clear();
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
		individualListView->insertItem( new MT_PopListViewItem(individualListView, actInd) );
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
	int popSize = individualListView->childCount();

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
	individualCountLCD->display( individualListView->childCount());
}

/***
 * displays the context menu if a right click on a individual occurs
 ***/ 
void MT_PopulationWidget::slotRButtonClicked(QListViewItem *item, const QPoint &point)
{
	popContextMenu->popup(point);
}

/***
 * checks if an individual is selected
 * enables/disables the delete function
 ***/
void MT_PopulationWidget::slotSelectionChanged()
{
	QListViewItem *actItem = individualListView->currentItem();
	if(individualListView->childCount() != 1 && actItem && actItem->isSelected())
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
void MT_PopulationWidget::slotCurrentChanged(QListViewItem *item)
{
	// clean the display
	individualProgramView->clear();

	// get the currently selected individual
	int pos = ((MT_PopListViewItem *) item)->getPos();
	MT_Individual *actInd = population->getIndividual(pos);

	// display the program
	for(int i=0; i<actInd->getProgram()->getLength(); i++){
		individualProgramView->insertLine(actInd->printProgramLine(i));
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

		QProgressDialog progress("Generating individuals", 0, number, this, 0, true);

		MT_Individual *newInd;
		MT_Randomizer *rand = gpManager->getRandomizer();
		for(int i=0; i<number; i++){
			progress.setProgress(i);
			int pos = population->createNewIndi(rand);
			newInd = population->getIndividual(pos);
			individualListView->insertItem( new MT_PopListViewItem(individualListView, newInd) );
			emit numChanged();
		}
		progress.setProgress(number);
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
	MT_PopListViewItem *actInd = (MT_PopListViewItem*) individualListView->firstChild();

	int DelPos =0;
	int ActPos =0;

	// iterate over all items, check if they are selected
	// and delete the selected ones
	MT_Individual *actRInd =0;
	while(actInd){
		nextInd = (MT_PopListViewItem*)actInd->nextSibling();	// get next individual in list
		if (( actInd->isSelected()) && (individualListView->childCount() != 1) )
		{
			DelPos = actInd->getPos();
			actRInd = population->delIndividual(DelPos);
			delete actRInd;
			emit numChanged();
			
			actIndNew = (MT_PopListViewItem*) individualListView->firstChild();
			while(actIndNew !=0)		// laufe über alle IndisItem  // für alle Item mit Pos > DelPos --> setPos(Pos-1)
			{
				ActPos = actIndNew->getPos();
				if (ActPos > DelPos)
					actIndNew->setPos(ActPos-1);
				
				actIndNew = (MT_PopListViewItem*)actIndNew->nextSibling();	// get next individual in list
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
	QStringList files( QFileDialog::getOpenFileNames( "Individuals(*.mind);;All Files(*)",
		0, this, "IndImpDialog", "Import Individuals"));

	if(!files.isEmpty()){
	
		// iterate over all selected files
		QStringList::Iterator it = files.begin();
		uint i = 0;
		uint count = files.count();
		QProgressDialog progress("Importing individuals", 0, count, this, 0, true);
		for( ; it != files.end(); ++it){
			QFile file(*it);
			progress.setProgress(i++);
			if(file.open(IO_ReadOnly)){		// file successfully opened
				QTextStream str(&file);

				//population->importPop(str);
				MT_Individual *newInd = new MT_Individual(str);
				population->addIndividual(newInd);
				individualListView->insertItem( new MT_PopListViewItem(individualListView, newInd) );
				
				 emit numChanged();
				file.close();
			}
		}
		progress.setProgress(count);
	}
}

/***
 * save the selected individual
 * if more than one is selected save them as a population or serial
 ***/
void MT_PopulationWidget::slotExpInd()
{
	QList<MT_PopListViewItem> *list = getSelectedItems();

	bool saveAsPop = false;
	QString fileName;
	if(list->count() > 1){
		saveAsPop = !QMessageBox::information(this, "Save individuals", "There are more than one individual selected.\n"
			"Shall we save them as a population?", "Save as population", "Save separately");
	}
	if(saveAsPop){
		fileName = QFileDialog::getSaveFileName(0, "Population Files (*.mpop);;All Files (*)", this);
		if(!fileName.isEmpty()){
			if(fileName.right(5) != ".mpop")
				fileName += ".mpop";
			QFile file( fileName );

			if(file.exists()){
				if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
					"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
					return;
			}

			if(file.open(IO_WriteOnly)){
				QTextStream str(&file);
				
				MT_Population npop;
				npop.changePopSize(0);//list->count());
				MT_PopListViewItem *actItem = list->first();
				for(int i=0; i<list->count(); i++){
					npop.addIndividual(population->getIndividual(actItem->getPos()));//, i);
					actItem = list->next();
				}

				npop.exportPop(str);
				npop.flush();
				

				file.close();
			} else {
				QMessageBox::critical(this, "Save population", "An error occured during saving the population.\nAborting operation.", "Ok");
			}
		}
	} else {
		fileName = QFileDialog::getSaveFileName(0, "Individual Files (*.mind);;All Files (*)", this);
		if(!fileName.isEmpty()){

			for(int i=0; i<list->count(); i++){
				QFile file( fileName.append("%1.mind").arg(i) );
				if(file.exists()){
					if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
						"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
						return;
				}
				if(file.open(IO_WriteOnly)){
					QTextStream str(&file);
					population->getIndividual(list->current()->getPos())->writeToFileIndi(str);
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
	QString fileName( QFileDialog::getOpenFileName(0, "Population Files (*.mpop);;All Files (*)", this) );

	if(!fileName.isEmpty()){
		QFile file( fileName );

		if(file.open(IO_ReadOnly)){
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
	QString fileName( QFileDialog::getSaveFileName(0, "Population Files (*.mpop);;All Files (*)", this) );

	if(!fileName.isEmpty()){
		if(fileName.right(5) != ".mpop")
			fileName += ".mpop";
		QFile file( fileName );

		if(file.exists()){
			if(0 == QMessageBox::warning(this, "Save population", "There is another file with this name. This will overwrite\n"
				"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
				return;
		}

		if(file.open(IO_WriteOnly)){
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
QList<MT_PopListViewItem> * MT_PopulationWidget::getSelectedItems()
{
	QList<MT_PopListViewItem> *lst = new QList<MT_PopListViewItem>;
	lst->setAutoDelete(false);
	QListViewItemIterator it(individualListView);
	for(; it.current(); ++it){
		if(it.current()->isSelected())
			lst->append((MT_PopListViewItem*)it.current());
	}
	return lst;
}