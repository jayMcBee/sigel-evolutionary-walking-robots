#include "MT_GUI/MT_PopulationWidget.h"

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
#include <QAbstractButton>


MT_PopulationWidget::MT_PopulationWidget(QMainWindow* parent, const char* name, Qt::WindowFlags fl)
	: MT_PopulationWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	oldPopSize = -1;
	mainWindow = static_cast<MT_MainWindow*>(parent);

	QString pixPath = ::getenv("SIGEL_ROOT");
	pixPath += "/pixmaps/";

	// create the toolbar
	popToolBar = new QToolBar(parent);
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
	addIndAction = new QAction(icon_addIndAction, "&Add...", parentWindow);
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
	impIndAction = new QAction(icon_impIndAction, "&Import...", parentWindow);
	impIndAction->setToolTip("Import Individual");
	impIndAction->setStatusTip("Imports one or more individuals from a file.");
	popToolBar->addAction(impIndAction);
	popContextMenu->addAction(impIndAction);

	QIcon icon_expIndAction(QPixmap(pixPath+"mt_ExpIndSmall.xpm"));
	icon_expIndAction.addPixmap(QPixmap(pixPath+"mt_ExpIndSmall.xpm"));
	expIndAction = new QAction(icon_expIndAction, "&Export...", parentWindow);
	expIndAction->setToolTip("Export Individual");
	expIndAction->setStatusTip("Writes the selected individuals to a file.");
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
	savePopAction->setStatusTip("Saves the complete population to a file.");
	popToolBar->addAction(savePopAction);

	// establish connections
	individualListView->setContextMenuPolicy(Qt::CustomContextMenu);
	QObject::connect(individualListView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotRButtonClicked(const QPoint&)));
	QObject::connect(individualListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotCurrentChanged(QTreeWidgetItem*)));
	QObject::connect(individualListView, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()));
	QObject::connect(addIndAction, SIGNAL(triggered()), SLOT(slotAddInd()));
	QObject::connect(delIndAction, SIGNAL(triggered()), SLOT(slotDelInd()));
	QObject::connect(impIndAction, SIGNAL(triggered()), SLOT(slotImpInd()));
	QObject::connect(expIndAction, SIGNAL(triggered()), SLOT(slotExpInd()));
	QObject::connect(loadPopAction, SIGNAL(triggered()), SLOT(slotLoadPop()));
	QObject::connect(savePopAction, SIGNAL(triggered()), SLOT(slotSavePop()));
	QObject::connect(this, SIGNAL(numChanged()), SLOT(slotNumChanged()));
};

/***
 * reads information from the gp-system and displays it
 ***/
void MT_PopulationWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	gpManager = manager;

	// clear the widget
	// Signals are blocked during clear(). Otherwise clear() reports a null
	// current item, and slotCurrentChanged empties the program view.
	{
		const bool wasBlocked = individualListView->blockSignals(true);
		individualListView->clear();
		individualListView->blockSignals(wasBlocked);
	}
	individualProgramView->setText("");
	individualCountLCD->display(0);

	// get the current population
	population = manager->getParent();
	if(!population)
	{
		QMessageBox::critical(this, "Configure MetaGP System", "Couldn't get current population.");
		return;
	}
	oldPopSize = population->getSize();

	// list all individuals
	MT_Individual *actInd;
	for(int i=0; i<population->getSize(); i++)
	{
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

	if(oldPopSize != popSize)
	{
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
		tournSize = mainWindow->calculateTournSize(popSize, offspringSize, tournSize);

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
	}
	else
	{
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

	if(!item)
		return;

	// get the currently selected individual
	int pos = static_cast<MT_PopListViewItem*>(item)->getPos();
	MT_Individual *actInd = population->getIndividual(pos);

	// display the program
	for(int i=0; i<actInd->getProgram()->getLength(); i++)
	{
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
	MT_AddIndividualsWidgetBase numDialog(this, nullptr, true);
	if(QDialog::Accepted != numDialog.exec())
		return;

	int number = numDialog.spinboxNumber->value();

	QProgressDialog progress("Generating individuals", QString(), 0, number, this);
	progress.setWindowModality(Qt::ApplicationModal);

	MT_Individual *newInd;
	MT_Randomizer *rand = gpManager->getRandomizer();
	for(int i=0; i<number; i++)
	{
		progress.setValue(i);
		int pos = population->createNewIndi(rand);
		newInd = population->getIndividual(pos);
		new MT_PopListViewItem(individualListView, newInd);
		emit numChanged();
	}
	progress.setValue(number);
}

/***
 * removes all selected individuals from the population, resizes the
 * population and deletes the removed individuals
 ***/
void MT_PopulationWidget::slotDelInd()
{
	int row = 0;
	while(row < individualListView->topLevelItemCount())
	{
		MT_PopListViewItem *item = static_cast<MT_PopListViewItem*>(individualListView->topLevelItem(row));

		// The last individual stays, also when it is selected.
		if(!item->isSelected() || individualListView->topLevelItemCount() == 1)
		{
			row++;
			continue;
		}

		int deletedPos = item->getPos();
		delete population->delIndividual(deletedPos);
		emit numChanged();

		// The individuals behind the deleted one move up by one.
		for(int i=0; i<individualListView->topLevelItemCount(); i++)
		{
			MT_PopListViewItem *otherItem = static_cast<MT_PopListViewItem*>(individualListView->topLevelItem(i));
			if(otherItem->getPos() > deletedPos)
				otherItem->setPos(otherItem->getPos() - 1);
		}

		// The next row takes this row's index, so row stays.
		delete item;
	}
}

/***
 * reads individuals from a file and appends them to the population
 ***/
void MT_PopulationWidget::slotImpInd()
{
	QStringList files( QFileDialog::getOpenFileNames( this, "Import Individuals", QString(), "Individuals (*.mind);;All Files (*)"));
	if(files.isEmpty())
		return;

	// iterate over all selected files
	QStringList::Iterator it = files.begin();
	uint i = 0;
	uint count = files.count();
	QProgressDialog progress("Importing individuals", QString(), 0, count, this);
	progress.setWindowModality(Qt::ApplicationModal);
	for( ; it != files.end(); ++it)
	{
		QFile file(*it);
		progress.setValue(i++);
		if(!file.open(QIODevice::ReadOnly))
			continue;

		QTextStream str(&file);

		MT_Individual *newInd = new MT_Individual(str);
		population->addIndividual(newInd);
		new MT_PopListViewItem(individualListView, newInd);

		emit numChanged();
		file.close();
	}
	progress.setValue(count);
}

/***
 * save the selected individual
 * if more than one is selected save them as a population or serial
 ***/
void MT_PopulationWidget::slotExpInd()
{
	QList<MT_PopListViewItem *> selectedItems = getSelectedItems();
	if(selectedItems.isEmpty())
		return;

	bool saveAsPop = false;
	if(selectedItems.count() > 1)
	{
		QMessageBox box(QMessageBox::Information, "Save Individuals", "There is more than one individual selected.\n"
		        "Shall we save them as a population?", QMessageBox::Yes | QMessageBox::No, this);
		box.button(QMessageBox::Yes)->setText("Save as population");
		box.button(QMessageBox::No)->setText("Save separately");
		box.setDefaultButton(QMessageBox::Yes);
		box.setEscapeButton(QMessageBox::No);
		saveAsPop = box.exec() == QMessageBox::Yes;
	}

	if(saveAsPop)
		exportAsPopulation(selectedItems);
	else
		exportAsIndividuals(selectedItems);
}

/***
 * writes the selected individuals into one population file
 ***/
void MT_PopulationWidget::exportAsPopulation(const QList<MT_PopListViewItem *> &selectedItems)
{
	QString fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)");
	if(fileName.isEmpty())
		return;

	if(fileName.right(5) != ".mpop")
		fileName += ".mpop";

	QFile file( fileName );
	if(!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this, "Save Population", "An error occurred while saving the population.\nThe operation is aborted.");
		return;
	}

	MT_Population npop;
	npop.changePopSize(0);
	for(int i=0; i<selectedItems.count(); i++)
	{
		MT_PopListViewItem *actItem = selectedItems.at(i);
		npop.addIndividual(population->getIndividual(actItem->getPos()));
	}

	QTextStream str(&file);
	npop.exportPop(str);
	npop.flush();

	file.close();
}

/***
 * writes each selected individual into its own file
 ***/
void MT_PopulationWidget::exportAsIndividuals(const QList<MT_PopListViewItem *> &selectedItems)
{
	QString fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Individual Files (*.mind);;All Files (*)");
	if(fileName.isEmpty())
		return;

	for(int i=0; i<selectedItems.count(); i++)
	{
		QFile file( fileName + QString("%1.mind").arg(i) );
		if(file.exists() && QMessageBox::Ok != QMessageBox::warning(this, "Save Population", "There is another file with this name. This will overwrite\n"
		        "the existing file. Do you really want to continue?", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel))
			return;

		if(!file.open(QIODevice::WriteOnly))
		{
			QMessageBox::critical(this, "Save Individual", "An error occurred while saving the individual.\nThe operation is aborted.");
			continue;
		}

		QTextStream str(&file);
		population->getIndividual(selectedItems.at(i)->getPos())->writeToFileIndi(str);
		file.close();
	}
}

/***
 * load a saved population
 ***/
void MT_PopulationWidget::slotLoadPop()
{
	QString fileName( QFileDialog::getOpenFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)") );
	if(fileName.isEmpty())
		return;

	QFile file( fileName );
	if(!file.open(QIODevice::ReadOnly))
		return;

	QMessageBox box(QMessageBox::Warning, "Import Population",
	        "Shall the current population be deleted or shall we append\n"
	        "the new individuals?", QMessageBox::Ok | QMessageBox::Discard, this);
	box.button(QMessageBox::Ok)->setText("Append");
	box.button(QMessageBox::Discard)->setText("Delete");
	box.setDefaultButton(QMessageBox::Ok);
	box.setEscapeButton(QMessageBox::Ok);

	QTextStream str(&file);
	if(box.exec() == QMessageBox::Discard)
		population->loadPop(str);
	else
		population->importPop(str);

	onShow(gpManager, nullptr);	// update the GUI
	file.close();
}

/***
 * export the current population to a file
 ***/
void MT_PopulationWidget::slotSavePop()
{
	QString fileName( QFileDialog::getSaveFileName(this, QString(), QString(), "Population Files (*.mpop);;All Files (*)") );
	if(fileName.isEmpty())
		return;

	if(fileName.right(5) != ".mpop")
		fileName += ".mpop";

	QFile file( fileName );
	if(!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this, "Save Population", "An error occurred while saving the population.\nThe operation is aborted.");
		return;
	}

	QTextStream str(&file);
	population->writeToFilePop(str);
	file.close();
}

/***
 * collects all selected items
 ***/
QList<MT_PopListViewItem *> MT_PopulationWidget::getSelectedItems()
{
	QList<MT_PopListViewItem *> lst;
	QTreeWidgetItemIterator it(individualListView);
	for(; (*it); ++it)
	{
		if((*it)->isSelected())
			lst.append(static_cast<MT_PopListViewItem*>(*it));
	}
	return lst;
}