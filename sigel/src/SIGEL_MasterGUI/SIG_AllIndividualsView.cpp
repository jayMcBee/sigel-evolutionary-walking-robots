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
#include <QTimer>
#include <QLineEdit>
#include <qapplication.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <QList>
#include <algorithm>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qlcdnumber.h>

#include <pvm3.h>
#ifndef _WINDOWS
#include <unistd.h>
#include <cstdlib>
#endif

#include "SIGEL_MasterGUI/SIG_AllIndividualsView.h"
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"
#include "SIGEL_MasterGUI/SIG_IndividualView.h"

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_GP/SIG_GPPVMData.h"

#include <SIGEL_Tools/SIG_IO.h>


namespace SIGEL_MasterGUI
{
  
SIG_AllIndividualsView::SIG_AllIndividualsView( QWidget * parent, const char * name, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : QSplitter( parent ), theExperiment( theExperiment )
{
  // change some splitter settings
  setOrientation( Qt::Vertical );
  setOpaqueResize( true );

  // create the upper part of the splitter (the individual list)
  individualList = new SIG_IndividualList( this, "IndividualListAndButtons" );

  // create the lower part of the splitter (the individual view)
  individualView = new SIG_IndividualView( this, "IndividualView" );

  // create the menu which pops up if one clicks on an individual
  individualMenu = new QMenu( this );
  individualMenu->addAction( "&Visualize", this, SLOT( slotVisualize() ) )->setShortcut( Qt::ALT | Qt::Key_V );
  individualMenu->addSeparator();
  individualMenu->addAction( "&Delete", this, SLOT( slotDeleteIndividuals() ) )->setShortcut( Qt::ALT | Qt::Key_D );
  individualMenu->addSeparator();
  individualMenu->addAction( "Import Program...", this, SLOT( slotImportProgram() ) )->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_K );
  individualMenu->addAction( "Export Program...", this, SLOT( slotExportProgram() ) )->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_K );
  individualMenu->addAction( "Export Individual...", this, SLOT( slotExportIndividual() ) )->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_L );
 
  // create the menu that pops up if one clicks on an empty spot in the list view
  listviewMenu = new QMenu( this );
  listviewMenu->addAction( "&Add...", this, SLOT( slotAddIndividuals() ) )->setShortcut( Qt::ALT | Qt::Key_A );
  listviewMenu->addSeparator();
  listviewMenu->addAction( "Import Individual...", this, SLOT( slotImportIndividual() ) )->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_L );

  // connect some stuff
  /* QObject::connect( individualList->pushbuttonAdd,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotAddIndividuals() ) );

  QObject::connect( individualList->pushbuttonDelete,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotDeleteIndividuals() ) );

  QObject::connect( individualList->pushbuttonStats,
		    SIGNAL( clicked() ),
		    this,
		    SLOT( slotStatsClicked() ) ); */
  
  // Qt 2's rightButtonClicked passed a GLOBAL position, column -1 for a click
  // that hit no item, and cleared the selection first (qlistview.cpp:3388-3396).
  individualList->listviewIndividuals->setContextMenuPolicy( Qt::CustomContextMenu );
  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( customContextMenuRequested( const QPoint & ) ),
		    this,
		    SLOT(slotRightButtonClicked( const QPoint & ) ) );
  
  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
		    this,
		    SLOT( slotDoubleClicked( QTreeWidgetItem * ) ) );

  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( itemSelectionChanged() ),
		    this,
		    SLOT( slotSelectionChanged() ) );

  
  /*
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * Perhaps also enable set AutoDelete!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */

  QList<int> valList;
  valList += 5;
  valList += 6;
  setSizes( valList );
};

SIG_AllIndividualsView::~SIG_AllIndividualsView()
{

};

int SIG_AllIndividualsView::numberOfSelectedItems()
{
  int numberOfSelectedItems = 0;

  // count the number of selected items
  QTreeWidgetItemIterator it( individualList->listviewIndividuals );
  for ( ; *it; ++it )
    {
      if( (*it)->isSelected() )
	{
	  numberOfSelectedItems++;
	}
    }
  return numberOfSelectedItems;
};

void SIG_AllIndividualsView::slotCompleteRefreshList()
{
  int poolSize = theExperiment.population.getSize();

  QProgressDialog progress( "Updating pool...", QString(), 0, poolSize, this );
  // Qt 2\'s trailing modal flag made it application modal.
  progress.setWindowModality( Qt::ApplicationModal );
  progress.setWindowTitle( "Updating" );
  progress.show();
  
  // clear() emits itemSelectionChanged while currentItem() still points into
  // the old pool, and getIndividual() calls exit(1) on a position that is gone.
  {
    const bool wasBlocked = individualList->listviewIndividuals->blockSignals( true );
    individualList->listviewIndividuals->clear();
    individualList->listviewIndividuals->blockSignals( wasBlocked );
  }
  for( int counter = 0; counter < poolSize; counter++ )
    {
      progress.setValue( counter );
      qApp->processEvents();
      SIGEL_GP::SIG_GPIndividual *theGPIndividual = &theExperiment.population.getIndividual( counter );
      SIG_IndividualListItem *theItem = new SIG_IndividualListItem( individualList->listviewIndividuals, counter, theGPIndividual );
    }
  individualList->lcdnumberNumberOfIndividuals->display(theExperiment.population.getSize() );
};

void SIG_AllIndividualsView::slotAddIndividuals()
{
  SIG_AddIndividualsDialog addDialog( this, "Add Individual Dialog", true );
  addDialog.spinboxNumber->setFocus();
  // The value must stay unselected, or a typed 2 replaces the 1.
  // Queued: the selection does not exist until exec() shows the dialog.
  if ( QLineEdit *le = addDialog.spinboxNumber->findChild<QLineEdit *>() )
    QTimer::singleShot( 0, le, [le]{ le->end( false ); } );
  switch( addDialog.exec() )
    {
    case QDialog::Accepted:
      emit signalDataRefreshNeeded();
      theExperiment.population.addRandomIndividuals( addDialog.spinboxNumber->value(), theExperiment.gpParameter, *theExperiment.robot.getLangParam() );
      
      // lets do it inefficiently first. will be corrected later
      // Same Qt 2 signal blocking as slotCompleteRefreshList above. Adding only
      // GROWS the pool, so a stale poolPosition still resolves to the same
      // individual today; the blocking matters the day this path stops being
      // append-only.
      {
        const bool wasBlocked = individualList->listviewIndividuals->blockSignals( true );
        individualList->listviewIndividuals->clear();
        individualList->listviewIndividuals->blockSignals( wasBlocked );
      }
      QProgressDialog progress( "Populating pool...", QString(), 0, theExperiment.population.getSize(), this );
  // Qt 2\'s trailing modal flag made it application modal.
  progress.setWindowModality( Qt::ApplicationModal );
      progress.setWindowTitle( "Add Individuals" );
      progress.show();
      for( int counter = 0; counter < theExperiment.population.getSize(); counter++ )
	{
	  progress.setValue( counter );
	  SIGEL_GP::SIG_GPIndividual *theGPIndividual = &theExperiment.population.getIndividual( counter );
	  SIG_IndividualListItem *theItem = new SIG_IndividualListItem( individualList->listviewIndividuals, counter, theGPIndividual );
	  qApp->processEvents();
	}
      break;
    }
  individualList->lcdnumberNumberOfIndividuals->display( theExperiment.population.getSize() );
};

void SIG_AllIndividualsView::slotDeleteIndividuals()
{
  /*
   * Delete works as follows. First we iterate over the list and save the position of each individual
   * that shall be deleted into an array. While iterating over the list each QTreeWidgetItem is deleted.
   * The the list gets sorted and every individual deleted. As the pool is rearranged each time an individual
   * gets deleted, we have to be careful that the right individual is deleted.
   */
  QList<int> positions;

  int numberOfSelectedItems = 0;

  // first count the number of selected items
  QTreeWidgetItemIterator it( individualList->listviewIndividuals );
  for ( ; *it; ++it )
    {
      if( (*it)->isSelected() )
	{
	  numberOfSelectedItems++;
	}
    }

  if( numberOfSelectedItems != 0 )
    {
      QString question;
      if( numberOfSelectedItems == 1 )
	question = "selected individual?";
      else
	question = QString::number( numberOfSelectedItems ) + " selected individuals?";
      
      switch( QMessageBox::warning( this, "Continue Deletion?" , "Do you really want to delete the " + question , QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	{
	case QMessageBox::Yes:
	  // resize the array, so we can save all positions
	  positions.resize( numberOfSelectedItems );
	  
	  int counter = 0;
	  
	  QTreeWidgetItemIterator it2( individualList->listviewIndividuals );
	  for ( ; *it2; ++it2 )
	    {
	      if( (*it2)->isSelected() )
		{
		  SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( (*it2) );
		  positions[ counter++ ] = individualListItem->poolPosition;
		}
	    }
	  
	  std::sort( positions.begin(), positions.end() );
	  
	  for( int count = 0; count < positions.size(); count++ )
	    {
	      theExperiment.population.deleteIndividual( positions[ count ] - count );
	    }
	  
	  slotCompleteRefreshList();
	  if( individualList->listviewIndividuals->topLevelItemCount() == 0 )
	    individualView->clear();
	  break;
	}
    } // if( numbeOfSelectedItems != 0 )
  
};

void SIG_AllIndividualsView::slotResetAllFitnessValues()
{
  theExperiment.population.resetAllFitnessValues();
  slotCompleteRefreshList();
};

void SIG_AllIndividualsView::slotStatsClicked()
{
  SIGEL_Tools::SIG_IO::cerr << "Pool Size:" << theExperiment.population.getSize() << Qt::endl;
};

void SIG_AllIndividualsView::slotRightButtonClicked( const QPoint &pos )
{
  QTreeWidget *theTree = individualList->listviewIndividuals;
  QTreeWidgetItem *theItem = theTree->itemAt( pos );
  const QPoint thePoint = theTree->viewport()->mapToGlobal( pos );
  if( !theItem ) // the click was outside
    {
      theTree->clearSelection();   // Qt 2 did this before emitting
      listviewMenu->popup( thePoint );
    }
  else
    {
      individualMenu->popup( thePoint );
    }
};    

void SIG_AllIndividualsView::slotDoubleClicked( QTreeWidgetItem *theItem )
{
  if( theItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( theItem );
      SIGEL_GP::SIG_GPIndividual *theGPIndividual = &theExperiment.population.getIndividual( individualListItem->poolPosition );
      SIG_IndividualView *theView = new SIG_IndividualView( 0, "IndividualViewDoubleClicked", Qt::WindowFlags(), theGPIndividual );
      // Qt 2 got this from WDestructiveClose
      theView->setAttribute( Qt::WA_DeleteOnClose );
      theView->show();
    }
};

void SIG_AllIndividualsView::slotVisualize()
{
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Starting visualization!" << Qt::endl;
#endif

  QTreeWidgetItemIterator it( individualList->listviewIndividuals );
  for ( ; *it; ++it ) {
    if( (*it)->isSelected() ) {

   	  SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( (*it) );
   	  SIGEL_GP::SIG_GPIndividual *theGPIndividual = individualListItem->theIndividual;

   	  emit signalDataRefreshNeeded();

   	  SIGEL_Program::SIG_Program &theProgram = theGPIndividual->getProgramVar();

   	  SIGEL_Robot::SIG_Robot modifiedRobot( theExperiment.robot );

   	  switch (theExperiment.simulationParameter.getSimulationLibrary())
   	  {
   	    case SIGEL_Simulation::SIG_SimulationParameters::DynaMo:
   	      try {
       		  modifiedRobot.prepareDynaMo();
       		}
       	  catch (SIGEL_Tools::SIG_Exception &e) {
       		  QMessageBox::warning( this, "Robot Exception", e.getMessage() );
       		  return;
       		};
   	      break;
   	    case SIGEL_Simulation::SIG_SimulationParameters::DynaMechs:
   	      try {
       		  modifiedRobot.prepareDynaMechs();
       		}
       	  catch (SIGEL_Tools::SIG_Exception &e) {
       		  QMessageBox::warning( this, "Robot Exception", e.getMessage() );
       		  return;
       		};
       	  break;
       };

   	  QList< char > hostNameBuffer( 100 );

   	  gethostname( hostNameBuffer.data(), 100 );

#ifdef _WINDOWS   	
   	  char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
   	  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif

   	  QString sigelRootString( sigelRootCString );

#ifdef _WINDOWS   	
   	  QString executableName = "sigel_slave";
#else
   	  QString executableName = sigelRootString + "/sigel_slave";
#endif   	
   	  QByteArray executableNameQCString = executableName.toUtf8();
   	  char const *executableNameCString = executableNameQCString;

   	  int taskId = 0;
   	  int spawnInfo = pvm_spawn( const_cast< char* >(executableNameCString),
   				     0,
   				     PvmTaskHost,
   				     hostNameBuffer.data(),
   				     1,
   				     &taskId );

   	  if (spawnInfo == 1) {
         SIGEL_GP::SIG_GPPVMData pvmData( modifiedRobot,
     			       theExperiment.environment,
     			       theExperiment.simulationParameter,
						 theExperiment.gpParameter.getFitnessName(),
     			       true );

         QString pvmDataString;
         QTextStream pvmDataStream( &pvmDataString, QIODevice::ReadWrite );
         pvmData.savePVMDataTransfer( pvmDataStream, theProgram );
         pvmData.sendQStringToPVM( pvmDataString, taskId, 23 );
   	  }
   	  else {
   	      QMessageBox::critical( this, "Error While Starting Slave", "The slave could not be started.\nEither there is no executable or a PVM problem occurred." );
   	  };
   	} // if-statement
  } //for-loop
};

void SIG_AllIndividualsView::slotSelectionChanged()
{
  /* QTreeWidgetItemIterator it( individualList->listviewIndividuals );
  for ( ; *it; ++it )
  { */
  QTreeWidgetItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      SIGEL_GP::SIG_GPIndividual *theGPIndividual = &theExperiment.population.getIndividual( individualListItem->poolPosition );
      individualView->textlabelShowName->setText( theGPIndividual->getName() );
      QString age = QString::number( theGPIndividual->getAge() );
      individualView->textlabelShowAge->setText( age );
      QString fitness = QString::number( theGPIndividual->getFitness() );
      individualView->textlabelShowFitness->setText( fitness );
      QString programCode;
      theGPIndividual->getProgramPointer()->printToString( programCode );
      individualView->multilineeditProgramCode->setText( programCode );
      QString history = theGPIndividual->getHistory().join( "\n" );
      individualView->multilineeditHistory->setText( history );
    }
};

void SIG_AllIndividualsView::slotEvolutionNotRunning( bool isNotRunning )
{
  if( !isNotRunning ) // means that the evolution is running...
    {
      individualView->clear();
      // Name the receiver. A wildcard also cuts QTreeWidget's own relays, so
      // itemDoubleClicked stops being emitted at all and slotDoubleClicked is
      // reconnected below to a signal nothing raises.
      QObject::disconnect( individualList->listviewIndividuals,
			   0,
			   this,
			   0 );
    }
  else
    {
      QObject::disconnect( individualList->listviewIndividuals,
			   0,
			   this,
			   0 );

      individualList->listviewIndividuals->setContextMenuPolicy( Qt::CustomContextMenu );
      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( customContextMenuRequested( const QPoint & ) ),
			this,
			SLOT(slotRightButtonClicked( const QPoint & ) ) );
      
      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
			this,
			SLOT( slotDoubleClicked( QTreeWidgetItem * ) ) );
      
      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( itemSelectionChanged() ),
			this,
			SLOT( slotSelectionChanged() ) );
    }
};

void SIG_AllIndividualsView::slotImportProgram()
{
  QTreeWidgetItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString fileName = QFileDialog::getOpenFileName( this, "Import Program", QString(), "Program Files (*.prg);;All Files (*)" );
	  if( !fileName.isEmpty() )
	    {
	      individualListItem->theIndividual->importProgram( fileName );
	    }
	  slotSelectionChanged();
	  slotCompleteRefreshList(); // can be done more efficiently!!!
	}
      else
	QMessageBox::information( this, "Select Exactly One Individual", "There must be exactly one individual selected." );
    } // if( currentitem )
  else
    QMessageBox::information( this, "No Individual Selected", "There is no individual selected." );
};

void SIG_AllIndividualsView::slotExportProgram()
{
  QTreeWidgetItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString individualName = individualListItem->theIndividual->getName();
	  QString fileName = QFileDialog::getSaveFileName( this, "Export Program", QString(), "Program Files (*.prg);;All Files (*)" );
	  if( !fileName.isEmpty() )
	    {
	      if( fileName.right(4) != ".prg" )
		fileName.append( ".prg" );
	      individualListItem->theIndividual->exportProgram( fileName );
	    }
	}
      else
	QMessageBox::information( this, "More Than One Individual Selected", "There is more than one individual selected." );
    }
  else
    QMessageBox::information( this, "No Individual Selected", "There is no individual selected." );
};

void SIG_AllIndividualsView::slotImportIndividual()
{
  QString fileName = QFileDialog::getOpenFileName( this, "Import Individual", QString(), "Individual Files (*.ind);;All Files (*)" );
  if( !fileName.isEmpty() )
    {
      theExperiment.population.importNewIndividual( fileName );
    }
  slotCompleteRefreshList(); // can be done more efficiently!!!
};

void SIG_AllIndividualsView::slotExportIndividual()
{
  QTreeWidgetItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString individualName = individualListItem->theIndividual->getName();
	  QString fileName = QFileDialog::getSaveFileName( this, "Export Individual", QString(), "Individual Files (*.ind);;All Files (*)" );
	  if( !fileName.isEmpty() )
	    {
	      if( fileName.right(4) != ".ind" )
		fileName.append( ".ind" );
	      individualListItem->theIndividual->exportIndividual( fileName );
	    }
	}
      else
	QMessageBox::information( this, "More Than One Individual Selected", "There is more than one individual selected." );
    }
  else
    QMessageBox::information( this, "No Individual Selected", "There is no individual selected." );
};

}
