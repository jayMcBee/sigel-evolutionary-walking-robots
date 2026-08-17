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
#include <qapplication.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qarray.h>
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
  : QSplitter( parent, name ), theExperiment( theExperiment )
{
  // change some splitter settings
  setOrientation( QSplitter::Vertical );
  setOpaqueResize( true );

  // create the upper part of the splitter (the individual list)
  individualList = new SIG_IndividualList( this, "IndividualListAndButtons" );

  // create the lower part of the splitter (the individual view)
  individualView = new SIG_IndividualView( this, "IndividualView" );

  // create the menu which pops up if one clicks on an individual
  individualMenu = new QPopupMenu( this );
  individualMenu->insertItem( "&Visualize", this, SLOT( slotVisualize() ), ALT+Key_V );
  individualMenu->insertSeparator();
  individualMenu->insertItem( "&Delete", this, SLOT( slotDeleteIndividuals() ), ALT+Key_D );
  individualMenu->insertSeparator();
  individualMenu->insertItem( "Import program", this, SLOT( slotImportProgram() ), CTRL+SHIFT+Key_K );
  individualMenu->insertItem( "Export program", this, SLOT( slotExportProgram() ), CTRL+ALT+Key_K );
  individualMenu->insertItem( "Export individual", this, SLOT( slotExportIndividual() ), CTRL+ALT+Key_L );
 
  // create the menu that pops up if one clicks on an empty spot in the list view
  listviewMenu = new QPopupMenu( this );
  listviewMenu->insertItem( "&Add", this, SLOT( slotAddIndividuals() ), ALT+Key_A );
  listviewMenu->insertSeparator();
  listviewMenu->insertItem( "Import individual", this, SLOT( slotImportIndividual() ), CTRL+SHIFT+Key_L );

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
  
  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( rightButtonClicked( QListViewItem *, const QPoint &, int ) ),
		    this,
		    SLOT(slotRightButtonClicked( QListViewItem *, const QPoint &, int ) ) );
  
  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( doubleClicked( QListViewItem * ) ),
		    this,
		    SLOT( slotDoubleClicked( QListViewItem * ) ) );

  QObject::connect( individualList->listviewIndividuals,
		    SIGNAL( selectionChanged() ),
		    this,
		    SLOT( slotSelectionChanged() ) );

  
  /*
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   * Eventuell noch set AutoDelete enablen!!!
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */

  QValueList<int> valList;
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
  QListViewItemIterator it( individualList->listviewIndividuals );
  for ( ; it.current(); ++it )
    {
      if( it.current()->isSelected() )
	{
	  numberOfSelectedItems++;
	}
    }
  return numberOfSelectedItems;
};

void SIG_AllIndividualsView::slotCompleteRefreshList()
{
  int poolSize = theExperiment.population.getSize();

  QProgressDialog progress( "Updating pool...", QString::null, poolSize, this, "progressDialogAdd", true );
  progress.setCaption( "Updating..." );
  progress.show();
  
  individualList->listviewIndividuals->clear();
  for( int counter = 0; counter < poolSize; counter++ )
    {
      progress.setProgress( counter );
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
  switch( addDialog.exec() )
    {
    case QDialog::Accepted:
      emit signalDataRefreshNeeded();
      theExperiment.population.addRandomIndividuals( addDialog.spinboxNumber->value(), theExperiment.gpParameter, *theExperiment.robot.getLangParam() );
      
      // lets do it inefficiently first. will be corrected later
      individualList->listviewIndividuals->clear();
      QProgressDialog progress( "Populating pool...", QString::null, theExperiment.population.getSize(), this, "progressDialogAdd", true );
      progress.setCaption( "Add individuals..." );
      progress.show();
      for( int counter = 0; counter < theExperiment.population.getSize(); counter++ )
	{
	  progress.setProgress( counter );
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
   * that shall be deleted into an array. While iterating over the list each QListViewItem is deleted.
   * The the list gets sorted and every individual deleted. As the pool is rearranged each time an individual
   * gets deleted, we have to be careful that the right individual is deleted.
   */
  QArray<int> positions;

  int numberOfSelectedItems = 0;

  // first count the number of selected items
  QListViewItemIterator it( individualList->listviewIndividuals );
  for ( ; it.current(); ++it )
    {
      if( it.current()->isSelected() )
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
      
      switch( QMessageBox::warning( 0, "Continue deletion?" , "Do you really want to delete the " + question , QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	{
	case QMessageBox::Yes:
	  // resize the array, so we can save all positions
	  positions.resize( numberOfSelectedItems );
	  
	  int counter = 0;
	  
	  QListViewItemIterator it2( individualList->listviewIndividuals );
	  for ( ; it2.current(); ++it2 )
	    {
	      if( it2.current()->isSelected() )
		{
		  SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( it2.current() );
		  positions[ counter++ ] = individualListItem->poolPosition;
		}
	    }
	  
	  positions.sort();
	  
	  for( int count = 0; count < positions.size(); count++ )
	    {
	      theExperiment.population.deleteIndividual( positions[ count ] - count );
	    }
	  
	  slotCompleteRefreshList();
	  if( individualList->listviewIndividuals->childCount() == 0 )
	    individualView->clear();
	  break;
	}
    } // if( numbeOfSelectedItems != 0 )
  
};

void SIG_AllIndividualsView::slotResetPool()
{
  theExperiment.population.resetPool();
  slotCompleteRefreshList();
};

void SIG_AllIndividualsView::slotStatsClicked()
{
  SIGEL_Tools::SIG_IO::cerr << "Pool Size:" << theExperiment.population.getSize() << endl;
};

void SIG_AllIndividualsView::slotRightButtonClicked( QListViewItem *theItem, const QPoint &thePoint, int inside )
{
  if( inside == -1) // the click was outside
    {
      listviewMenu->popup( thePoint );
    }
  else
    {
      individualMenu->popup( thePoint );
    }
};    

void SIG_AllIndividualsView::slotDoubleClicked( QListViewItem *theItem )
{
  if( theItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( theItem );
      SIGEL_GP::SIG_GPIndividual *theGPIndividual = &theExperiment.population.getIndividual( individualListItem->poolPosition );
      SIG_IndividualView *theView = new SIG_IndividualView( 0, "IndividualViewDoubleClicked", WDestructiveClose, theGPIndividual );
      theView->show();
    }
};

void SIG_AllIndividualsView::slotVisualize()
{
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Starting visualization!\n";
#endif

  QListViewItemIterator it( individualList->listviewIndividuals );
  for ( ; it.current(); ++it ) {
    if( it.current()->isSelected() ) {

   	  SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( it.current() );
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
       		  QMessageBox::warning( this, "Robot Exception!", e.getMessage() );
       		  return;
       		};
   	      break;
   	    case SIGEL_Simulation::SIG_SimulationParameters::DynaMechs:
   	      try {
       		  modifiedRobot.prepareDynaMechs();
       		}
       	  catch (SIGEL_Tools::SIG_Exception &e) {
       		  QMessageBox::warning( this, "Robot Exception!", e.getMessage() );
       		  return;
       		};
       	  break;
       };

   	  QArray< char > hostNameBuffer( 100 );

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
   	  QCString executableNameQCString = executableName.utf8();
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
         QTextStream pvmDataStream( &pvmDataString, IO_ReadWrite );
         pvmData.savePVMDataTransfer( pvmDataStream, theProgram );
         pvmData.sendQStringToPVM( pvmDataString, taskId, 23 );
   	  }
   	  else {
   	      QMessageBox::critical( this, "Error while starting slave!", "The slave could not be started.\n Either there is not executable or a PVM problem occured." );
   	  };
   	} // if-statement
  } //for-loop
};

void SIG_AllIndividualsView::slotSelectionChanged()
{
  /* QListViewItemIterator it( individualList->listviewIndividuals );
  for ( ; it.current(); ++it )
  { */
  QListViewItem *currentItem = individualList->listviewIndividuals->currentItem();
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
      QObject::disconnect( individualList->listviewIndividuals,
			   0,
			   0,
			   0 );
    }
  else
    {
      QObject::disconnect( individualList->listviewIndividuals,
			   0,
			   0,
			   0 );

      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( rightButtonClicked( QListViewItem *, const QPoint &, int ) ),
			this,
			SLOT(slotRightButtonClicked( QListViewItem *, const QPoint &, int ) ) );
      
      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( doubleClicked( QListViewItem * ) ),
			this,
			SLOT( slotDoubleClicked( QListViewItem * ) ) );
      
      QObject::connect( individualList->listviewIndividuals,
			SIGNAL( selectionChanged() ),
			this,
			SLOT( slotSelectionChanged() ) );
    }
};

void SIG_AllIndividualsView::slotImportProgram()
{
  QListViewItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString fileName = QFileDialog::getOpenFileName( QString::null, "Program files (*.prg);;All Files (*)", 0, 0, "Import program..." );
	  if( !fileName.isEmpty() )
	    {
	      individualListItem->theIndividual->importProgram( fileName );
	    }
	  slotSelectionChanged();
	  slotCompleteRefreshList(); // can be done more efficiently!!!
	}
      else
	QMessageBox::information( 0, "Select exactly one individual...", "There must be exactly one individual selected!" );
    } // if( currentitem )
  else
    QMessageBox::information( 0, "No individual selected...", "There is no individual selected!" );
};

void SIG_AllIndividualsView::slotExportProgram()
{
  QListViewItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString individualName = individualListItem->theIndividual->getName();
	  QString fileName = QFileDialog::getSaveFileName( QString::null, "Program files (*.prg);;All Files (*)", 0, 0, "Export program..." );
	  if( !fileName.isEmpty() )
	    {
	      if( fileName.right(4) != ".prg" )
		fileName.append( ".prg" );
	      individualListItem->theIndividual->exportProgram( fileName );
	    }
	}
      else
	QMessageBox::information( 0, "More than one individual selected...", "There is more than one individual selected!" );
    }
  else
    QMessageBox::information( 0, "No individual selected...", "There is no individual selected!" );
};

void SIG_AllIndividualsView::slotImportIndividual()
{
  QString fileName = QFileDialog::getOpenFileName( QString::null, "Individual files (*.ind);;All Files (*)", 0, 0, "Import individual..." );
  if( !fileName.isEmpty() )
    {
      theExperiment.population.importNewIndividual( fileName );
    }
  slotCompleteRefreshList(); // can be done more efficiently!!!
};

void SIG_AllIndividualsView::slotExportIndividual()
{
  QListViewItem *currentItem = individualList->listviewIndividuals->currentItem();
  if( currentItem )
    {
      SIG_IndividualListItem *individualListItem = static_cast<SIG_IndividualListItem *> ( currentItem );
      if( numberOfSelectedItems() == 1 )
	{
	  QString individualName = individualListItem->theIndividual->getName();
	  QString fileName = QFileDialog::getSaveFileName( QString::null, "Individual files (*.ind);;All Files (*)", 0, 0, "Export individual..." );
	  if( !fileName.isEmpty() )
	    {
	      if( fileName.right(4) != ".ind" )
		fileName.append( ".ind" );
	      individualListItem->theIndividual->exportIndividual( fileName );
	    }
	}
      else
	QMessageBox::information( 0, "More than one individual selected...", "There is more than one individual selected!" );
    }
  else
    QMessageBox::information( 0, "No individual selected...", "There is no individual selected!" );
};

}
