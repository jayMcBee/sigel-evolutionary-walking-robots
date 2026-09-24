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
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfileinfo.h>

#include "SIGEL_MasterGUI/SIG_RenameDialog.h"
#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"
#include "SIGEL_MasterGUI/SIG_ExperimentItem.h"

#include "SIGEL_GP/SIG_GUIGPManager.h"

#include <SIGEL_Tools/SIG_IO.h>

namespace SIGEL_MasterGUI
{

  SIG_ExperimentListView::SIG_ExperimentListView( QWidget * parent, const char * name, QStackedWidget *theWidgetStack ) : QTreeWidget( parent ), widgetStack( theWidgetStack ), numberOfExperiments(1)
{
  setHeaderLabels( QStringList( "Experiments" ) );
  setContextMenuPolicy( Qt::CustomContextMenu );
  QObject::connect( this,
		    SIGNAL( customContextMenuRequested( const QPoint & ) ),
		    SLOT( slotRightButtonClicked( const QPoint & ) ) );
  QObject::connect( this,
		    SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
		    SLOT( slotSelectionChanged( QTreeWidgetItem * ) ));
  experimentListViewMenu = new QMenu( this );
  experimentListViewMenu->setObjectName( "SIG_ExperimentListViewPopupMenu" );

  setRootIsDecorated( true );
  // Qt 2: setSorting(-1) meant "do not sort at all".
  setSortingEnabled( false );
  
};

SIG_ExperimentListView::~SIG_ExperimentListView()
{
  qDeleteAll( experimentDict );
};

void SIG_ExperimentListView::slotNewExperiment()
{
  QString project;
  project.setNum( numberOfExperiments++ );
  project.prepend("Experiment-");
  project.append(".exp");
  if( experimentExists( project ) )
    project = getAlternativeName( project );
  SIG_ExperimentItem *theNewItem = new SIG_ExperimentItem( this, project );
  SIG_GUIGPExperiment *theNewExperiment = new SIG_GUIGPExperiment( project, widgetStack, theNewItem );
  QObject::connect( theNewExperiment,
		    SIGNAL( signalEvolutionNotRunning( bool ) ),
		    this,
		    SIGNAL( evolutionNotRunning( bool ) ) );
  experimentDict.insert( project , theNewExperiment );
  emit isNotEmpty( true );
  if ( theNewItem )
	    setCurrentItem( theNewItem );
};

void SIG_ExperimentListView::slotRenameExperiment()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      SIG_RenameDialog renameDialog( this, 0, true );
      QString oldName = currentlySelectedExperimentName();
      renameDialog.setWindowTitle( "Rename " + oldName );
      renameDialog.lineeditNewName->setText( oldName );
      renameDialog.lineeditNewName->selectAll();
      renameDialog.lineeditNewName->setFocus();
      switch( renameDialog.exec() )
	{
	case QDialog::Accepted:
	  QString newName = renameDialog.lineeditNewName->text();
	  // we got a real new name so lets do it
	  if( newName != QString() && newName != oldName && !experimentExists( newName ) )
	    {
	      // the string is not empty, was really changed and there is no other experiment with that name, so lets go

	      // append .exp if it wasn't entered
	      if( newName.right( 4 ) != ".exp")
		newName.append( ".exp" );

	      // disable autodelete to keep the SIG_GUIGPExperiment object
	      experimentDict.take( oldName );

	      theExperiment->setName( newName );
	      // insert the experiment into the experiment dictionary
	      experimentDict.insert( newName, theExperiment );
      
	      // enable autodelete
	    } // close if
	  else
	    QMessageBox::information( this, "Error", "Either you have entered no name or there is\nalready an experiment under that name." );
	  break;
	} // close switch
    } // close if
};

void SIG_ExperimentListView::slotDeleteExperiment()
{
  // Find out which experiment is selected...
  QTreeWidgetItem *current = currentItem();
  if(current)
    {
      while(current->parent() != 0)
	{
	  current = current->parent();
	}
      QString name = current->text(0);
      // lets ask first if we really want to do this
      
      switch( QMessageBox::warning( this, "Delete Experiment", "Do you really want to delete\n"
				    "the experiment " + name + "?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No) )
	{
	case QMessageBox::Yes:
	  // Qt 2 ran this with autoDelete ON: the removed value was deleted
	  delete experimentDict.take( name );
	  takeTopLevelItem( indexOfTopLevelItem( current ) );
	  delete current;
	  if ( QTreeWidgetItem *firstItem = topLevelItem( 0 ) )
	    setCurrentItem( firstItem );
	  break;
	};
    }
  else
    {
      QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
    };
  if( topLevelItemCount() == 0 )
    {
      emit isNotEmpty( false );
    }
  else
    {
      emit isNotEmpty( true );
    }
};

void SIG_ExperimentListView::slotLoadExperiment()
{
  QStringList filesToOpen = QFileDialog::getOpenFileNames( this, "Load Experiments", QString(), "Experiment Files (*.exp);;All Files (*)" );
  if( !filesToOpen.isEmpty() )
    {
      for( int i = 0; i < filesToOpen.count(); i++)
	{
	  QString absFileName = filesToOpen[i];
	  int slashPosition = absFileName.lastIndexOf( "/" );
	  QString fileName = absFileName.right( absFileName.length() - (slashPosition + 1) );
	  if ( experimentExists( fileName ) )
	    fileName = getAlternativeName( fileName );
	  SIG_ExperimentItem *theNewItem = new SIG_ExperimentItem( this, fileName );

	  // SIG_GPPopulation::readFromFile and SIG_AllIndividualsView::slotCompleteRefreshList
	  // run processEvents below, before this name is in experimentDict. Qt 6
	  // makes even a non-selectable item current on focus-in; a disabled one not.
	  theNewItem->setExpanded(false);
	  theNewItem->setFlags( theNewItem->flags() & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) );

	  SIG_GUIGPExperiment *theNewExperiment = new SIG_GUIGPExperiment( fileName, widgetStack, theNewItem );
	  QObject::connect( theNewExperiment,
			    SIGNAL( signalEvolutionNotRunning( bool ) ),
			    this,
			    SIGNAL( evolutionNotRunning( bool ) ) );
	  
	  QFile file( absFileName );
	  if( file.open(QIODevice::ReadOnly) )
	    {
	      QTextStream theStream( &file );
	      // this is for the autosave function
	      // so the gpExperiment knows where to save the experiment
	      theNewExperiment->gpExperiment.setPath(absFileName);
	      theNewExperiment->gpExperiment.loadExperiment( theStream );
	      theNewExperiment->getAllOutOfExperiment();
	    }
	  else
	    {
	      QMessageBox::warning( this, "File could not be opened", "The file " + fileName + " could not be opened for reading." );
	    }
	  experimentDict.insert( fileName , theNewExperiment );
	  emit isNotEmpty( true );

	  theNewItem->setFlags( theNewItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	  theNewItem->setExpanded(true);
	  
	} // for each filename end
      if ( QTreeWidgetItem *firstItem = topLevelItem( 0 ) )
	    setCurrentItem( firstItem );
    };
};

void SIG_ExperimentListView::slotSaveExperiment()
{
  QString currentExperiment = currentlySelectedExperimentName();
  if (currentExperiment != QString() ) {
      SIG_GUIGPExperiment *theExperiment = getByExperimentName( currentExperiment );
      theExperiment->putAllIntoExperiment();
      QString fileName = QFileDialog::getSaveFileName( this, "Save Experiment", theExperiment->getName(), "Experiment Files (*.exp);;All Files (*)" );
      if( !fileName.isEmpty() ) {
	  fileName = theExperiment->checkEnding( fileName, "exp" );
	  QFile file( fileName );
	  if( file.open(QIODevice::WriteOnly) ) {
	      QTextStream theStream( &file );
	      theExperiment->gpExperiment.saveExperiment( theStream );
	  }
	  file.close();

	  QFileInfo fileInfo( fileName );

	  experimentDict.take( currentExperiment );

	  theExperiment->setName( fileInfo.fileName() );

	  experimentDict.insert( fileInfo.fileName(), theExperiment );
	
	  // this is for the autosave function
	  // so the gpExperiment knows where to save the experiment
		theExperiment->gpExperiment.setPath(fileName);
		} // filename not empty
  } // save selected experiment
  else	{ // no experiment selected
  	QMessageBox::warning( this, "No experiment selected", "Currently there is no experiment selected!" );
  }
};

void SIG_ExperimentListView::slotRightButtonClicked( const QPoint & pos )
{
  QTreeWidgetItem *theItem = itemAt( pos );
  const QPoint thePoint = viewport()->mapToGlobal( pos );
  if ( !theItem )
    {
      clearSelection();                        // Qt 2 did this before emitting
      experimentListViewMenu->popup( thePoint ); // the click was outside
    }
  else
    {
      QString option = theItem->text(0);
      QString experimentName;
      while( theItem->parent() != 0)
	{
	  theItem = theItem->parent();
	}
      experimentName = theItem->text(0);
      if( SIG_GUIGPExperiment *theExperiment = experimentDict.value( experimentName ) )
	theExperiment->slotRightClick( option, thePoint );
    }
};


void SIG_ExperimentListView::slotSelectionChanged( QTreeWidgetItem * theItem )
{
  if(theItem)
    {
      QString option = theItem->text(0);
      QString experimentName;
      while( theItem->parent() != 0)
	{
	  theItem = theItem->parent();
	}
      experimentName = theItem->text(0);
      // slotLoadExperiment builds the item before the experiment exists.
      SIG_GUIGPExperiment *theExperiment = experimentDict.value( experimentName );
      if( !theExperiment )
	return;
      theExperiment->slotSelectionChanged( option );
      // ANY run, not the clicked experiment's. The tree itself is never
      // locked, so a mid-run click can select a not-running experiment; asking
      // that one would hand back every locked action.
      emit evolutionNotRunning( !isRunning() );
 	  emit currentExperimentChanged();
      theExperiment->putAllIntoExperiment();
    }
};

SIG_GUIGPExperiment* SIG_ExperimentListView::getByExperimentName( QString name )
{
  if( name != QString() )
    return experimentDict.value( name );
  return NULL;
};

bool SIG_ExperimentListView::experimentExists( QString name )
{
  if( experimentDict.value( name ) )
    return true;
  else
    return false;
};

QString SIG_ExperimentListView::getAlternativeName( QString existingName )
{
  int number = 1;
  QString alternativeName = existingName;
  while ( experimentExists( alternativeName ) )
    {
      alternativeName = "(" + QString::number( number++ ) + ")" + existingName;
    }
  return alternativeName; // has to be implemented right!
};

SIG_GUIGPExperiment* SIG_ExperimentListView::currentlySelectedExperiment()
{
  QString experimentName = currentlySelectedExperimentName();
  if( experimentName != QString() )
    return experimentDict.value( experimentName );
  else
    return 0;
};

bool SIG_ExperimentListView::isRunning()
{
  for ( SIG_GUIGPExperiment *theExperiment : experimentDict )
    if ( theExperiment->isRunning() )
      return true;
  return false;
};

QString SIG_ExperimentListView::currentlySelectedExperimentName()
{
  QTreeWidgetItem *theCurrentItem = currentItem();
  if (theCurrentItem)
    {
      while( theCurrentItem->parent() != 0)
	{
	  theCurrentItem = theCurrentItem->parent();
	}
      return theCurrentItem->text( 0 );
    }
  else
    return QString();
};

void SIG_ExperimentListView::selectItem( QString label )
{
  QTreeWidgetItem *theItem = currentItem();
  if( theItem )
    {
      while( theItem->parent() != 0)
	{
	  theItem = theItem->parent();
	}

      for(; theItem; theItem = itemBelow( theItem ) )
	{
	  if( theItem->text( 0 ) == label )
	    {
	      if ( theItem )
	    setCurrentItem( theItem );
	      break;
	    }
	}

      /* QTreeWidgetItemIterator it( theItem );
       * for ( ; *it; it++ )
       * 	{
       *	  if( (*it)->text( 0 ) == label )
       *    {
       *	      setSelected( (*it), true );
       *	      break;
       *	    }
       *	} */
    }
  else
    {
      QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
    }
};

void SIG_ExperimentListView::slotShowGPParameters()
{
  selectItem( "GP-Parameters" );
};

void SIG_ExperimentListView::slotShowSimulationParameters()
{
  selectItem( "Simulation-Parameters" );
};

void SIG_ExperimentListView::slotShowLanguageParameters()
{
  selectItem( "Language-Parameters" );
};

void SIG_ExperimentListView::slotShowRobot()
{
  selectItem( "Robot" );
};

void SIG_ExperimentListView::slotShowEnvironment()
{
  selectItem( "Environment" );
};
  
void SIG_ExperimentListView::slotShowIndividuals()
{
  selectItem( "Individuals" );
};

void SIG_ExperimentListView::slotGPParametersImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowGPParameters();
      theExperiment->slotGPParameterImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};
  
void SIG_ExperimentListView::slotSimulationParametersImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowSimulationParameters();
      theExperiment->slotSimulationParameterImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotRobotImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowRobot();
      theExperiment->slotRobotImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotLanguageParametersImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowLanguageParameters();
      theExperiment->slotLanguageParameterImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotPopulationImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->slotPopulationImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotEnvironmentImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowEnvironment();
      theExperiment->slotEnvironmentImport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotGPParametersExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowGPParameters();
      theExperiment->slotGPParameterExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotSimulationParametersExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowSimulationParameters();
      theExperiment->slotSimulationParameterExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotLanguageParametersExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowLanguageParameters();
      theExperiment->slotLanguageParameterExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotPopulationExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->slotPopulationExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotEnvironmentExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowEnvironment();
      theExperiment->slotEnvironmentExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotGNUPlotExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      theExperiment->slotGNUPlotExport();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotAddIndividuals()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotAddIndividuals();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotDeleteIndividuals()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotDeleteIndividuals();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotResetAllFitnessValues()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotResetAllFitnessValues();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotVisualizeIndividuals()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( !theExperiment )
    {
      QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
      return;
    }

  // write all changes back to generate a new dynamechs terrain
  theExperiment->putAllIntoExperiment();
  slotShowIndividuals();
  theExperiment->allIndividualsView->slotVisualize();
};

void SIG_ExperimentListView::slotProgramExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotExportProgram();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotIndividualExport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotExportIndividual();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotProgramImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotImportProgram();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotIndividualImport()
{
  SIG_GUIGPExperiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotImportIndividual();
    }
  else
    QMessageBox::information( this, "No experiment selected", "Currently there is no experiment selected." );
};

} // close namespace

