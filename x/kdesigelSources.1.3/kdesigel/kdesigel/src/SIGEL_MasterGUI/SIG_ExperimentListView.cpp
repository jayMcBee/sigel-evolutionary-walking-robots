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

  SIG_ExperimentListView::SIG_ExperimentListView( QWidget * parent, const char * name, QWidgetStack *theWidgetStack ) : QListView( parent, name ), widgetStack( theWidgetStack ), numberOfExperiments(1)
{
  addColumn( "Experiments" );
  QObject::connect( this,
		    SIGNAL( rightButtonClicked ( QListViewItem *, const QPoint &, int ) ),
		    SLOT( slotRightButtonClicked( QListViewItem *, const QPoint &, int ) ) );
  QObject::connect( this,
		    SIGNAL( selectionChanged( QListViewItem * ) ),
		    SLOT( slotSelectionChanged( QListViewItem * ) ));
  experimentListViewMenu = new QPopupMenu( this, "SIG_ExperimentListViewPopupMenu" );

  setRootIsDecorated( TRUE );
  setSorting( -1 );
  
  experimentDict.setAutoDelete( true );
};

SIG_ExperimentListView::~SIG_ExperimentListView(){};

void SIG_ExperimentListView::slotNewExperiment()
{
  QString project;
  project.setNum( numberOfExperiments++ );
  project.prepend("Experiment-");
  project.append(".exp");
  if( experimentExists( project ) )
    project = getAlternativeName( project );
  SIG_ExperimentItem *theNewItem = new SIG_ExperimentItem( this, project );
  SIG_Experiment *theNewExperiment = new SIG_Experiment( project, widgetStack, theNewItem );
  QObject::connect( theNewExperiment,
		    SIGNAL( signalEvolutionNotRunning( bool ) ),
		    this,
		    SIGNAL( evolutionNotRunning( bool ) ) );
  experimentDict.insert( project , theNewExperiment );
  emit isNotEmpty( true );
  this->setSelected( theNewItem, true );
};

void SIG_ExperimentListView::slotRenameExperiment()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      SIG_RenameDialog renameDialog( this, 0, true );
      QString oldName = currentlySelectedExperimentName();
      renameDialog.setCaption( "Rename " + oldName );
      renameDialog.lineeditNewName->setText( oldName );
      renameDialog.lineeditNewName->selectAll();
      renameDialog.lineeditNewName->setFocus();
      switch( renameDialog.exec() )
	{
	case QDialog::Accepted:
	  QString newName = renameDialog.lineeditNewName->text();
	  // we got a real new name so lets do it
	  if( newName != QString::null && newName != oldName && !experimentExists( newName ) )
	    {
	      // the string is not empty, was really changed and there is no other experiment with that name, so lets go

	      // append .exp if it wasn't entered
	      if( newName.right( 4 ) != ".exp")
		newName.append( ".exp" );

	      // disable autodelete to keep the SIG_Experiment object
	      experimentDict.setAutoDelete( false );
	      experimentDict.remove( oldName );

	      theExperiment->setName( newName );
	      // insert the experiment into the experiment dictionary
	      experimentDict.insert( newName, theExperiment );
      
	      // enable autodelete
	      experimentDict.setAutoDelete( true );
	    } // close if
	  else
	    QMessageBox::information( this, "Error...", "Either you have entered no name or there is\nalready an experiment under that name." );
	  break;
	} // close switch
    } // close if
};

void SIG_ExperimentListView::slotDeleteExperiment()
{
  // Find out which experiment is selected...
  QListViewItem *current = currentItem();
  if(current)
    {
      while(current->parent() != 0)
	{
	  current = current->parent();
	}
      QString name = current->text(0);
      // lets ask first if we really want to do this
      
      switch( QMessageBox::warning( this, "Do you really...", "Do you really want to delete\n"
				    "the experiment " + name + "?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No) )
	{
	case QMessageBox::Yes:
	  experimentDict.remove( name );
	  takeItem(current);
	  delete current;
	  setSelected( firstChild(), true );
	  break;
	};
    }
  else
    {
      QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
    };
  if( this->childCount() == 0 )
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
  QStringList filesToOpen = QFileDialog::getOpenFileNames( "Experiment Files (*.exp);;All Files (*)", QString::null, 0, "filesToOpen", "Load Experiments..." );
  if( !filesToOpen.isEmpty() )
    {
      for( int i = 0; i < filesToOpen.count(); i++)
	{
	  QString absFileName = filesToOpen[i];
	  int slashPosition = absFileName.findRev( "/" );
	  QString fileName = absFileName.right( absFileName.length() - (slashPosition + 1) );
	  if ( experimentExists( fileName ) )
	    fileName = getAlternativeName( fileName );
	  SIG_ExperimentItem *theNewItem = new SIG_ExperimentItem( this, fileName );

	  // lets test something
	  theNewItem->setOpen(false);
	  theNewItem->setSelectable( false );

	  SIG_Experiment *theNewExperiment = new SIG_Experiment( fileName, widgetStack, theNewItem );
	  QObject::connect( theNewExperiment,
			    SIGNAL( signalEvolutionNotRunning( bool ) ),
			    this,
			    SIGNAL( evolutionNotRunning( bool ) ) );
	  
	  QFile file( absFileName );
	  if( file.open(IO_ReadOnly) )
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
	      QMessageBox::warning( 0, "File could not be opened...", "The file " + fileName + "could not be opened for reading." );
	    }
	  experimentDict.insert( fileName , theNewExperiment );
	  emit isNotEmpty( true );

	  // lets test something 2
	  theNewItem->setSelectable( true );
	  theNewItem->setOpen(true);
	  
	} // for each filename end
      setSelected( firstChild(), true );
    };
};

void SIG_ExperimentListView::slotSaveExperiment()
{
  QString currentExperiment = currentlySelectedExperimentName();
  if (currentExperiment != QString::null ) {
      SIG_Experiment *theExperiment = getByExperimentName( currentExperiment );
      theExperiment->putAllIntoExperiment();
      QString fileName = QFileDialog::getSaveFileName( theExperiment->getName(), "Experiment Files (*.exp);;All Files (*)", 0, "ExperimentSaveDialog", "Save Experiment...");
      if( !fileName.isEmpty() ) {
	  		if( fileName.right(4) != ".exp" ) fileName.append( ".exp" );
	  		QFile file( fileName );
	  		if( file.exists() )
	    		switch( QMessageBox::warning( 0, "File exists...", "The file " + file.name() + " exists!\nDo you want to overwrite?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape ) )
	      	{
	      		case QMessageBox::Yes:
							if( file.open(IO_WriteOnly) ) {
		    				QTextStream theStream( &file );
		    				theExperiment->gpExperiment.saveExperiment( theStream );
		  		    }
				    file.close();

				    QFileInfo fileInfo( fileName );

						experimentDict.setAutoDelete( false );
						experimentDict.remove( currentExperiment );

						theExperiment->setName( fileInfo.fileName() );

						experimentDict.insert( fileInfo.fileName(), theExperiment );
      
						experimentDict.setAutoDelete( true );

						break;
	      	} // switch
	  	else { // file does not exist
	      if( file.open(IO_WriteOnly) ) {
		  		QTextStream theStream( &file );
		  		theExperiment->gpExperiment.saveExperiment( theStream );
				}
	      file.close();

	      QFileInfo fileInfo( fileName );

	      experimentDict.setAutoDelete( false );
	      experimentDict.remove( currentExperiment );

	      theExperiment->setName( fileInfo.fileName() );

	      experimentDict.insert( fileInfo.fileName(), theExperiment );
      
	      experimentDict.setAutoDelete( true );
	    } // else (file does not exist)
	
	  // this is for the autosave function
	  // so the gpExperiment knows where to save the experiment
		theExperiment->gpExperiment.setPath(fileName);
		} // filename not empty
  } // save selected experiment
  else	{ // no experiment selected
  	QMessageBox::warning( this, "No experiment selected...", "Currently there is no experiment selected!" );
  }
};

void SIG_ExperimentListView::slotRightButtonClicked( QListViewItem * theItem, const QPoint & thePoint, int inside )
{
  if (inside==-1)
    experimentListViewMenu->popup( thePoint ); // the click was outside
  else
    {
      QString option = theItem->text(0);
      QString experimentName;
      while( theItem->parent() != 0)
	{
	  theItem = theItem->parent();
	}
      experimentName = theItem->text(0);
      experimentDict[ experimentName ]->slotRightClick( option, thePoint );
    }
};


void SIG_ExperimentListView::slotSelectionChanged( QListViewItem * theItem )
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
      experimentDict[ experimentName ]->slotSelectionChanged( option );
      SIGEL_GP::SIG_GUIGPManager *manager = experimentDict[ experimentName ]->gpManager;
      if( manager )
	{
	  if( manager->running() )
	    emit evolutionNotRunning( false );
	  else
	    emit evolutionNotRunning( true );
	}
      else
	emit evolutionNotRunning( true );
 	  emit actExpChanged();
      experimentDict[ experimentName ]->putAllIntoExperiment();
    }
};

SIG_Experiment* SIG_ExperimentListView::getByExperimentName( QString name )
{
  if( name != QString::null )
    return experimentDict[ name ];
  return NULL;
};

bool SIG_ExperimentListView::experimentExists( QString name )
{
  if( experimentDict[ name ] )
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

SIG_Experiment* SIG_ExperimentListView::currentlySelectedExperiment()
{
  QString experimentName = currentlySelectedExperimentName();
  if( experimentName != QString::null )
    return experimentDict[ experimentName ];
  else
    return 0;
};

QString SIG_ExperimentListView::currentlySelectedExperimentName()
{
  QListViewItem *theCurrentItem = currentItem();
  if (theCurrentItem)
    {
      while( theCurrentItem->parent() != 0)
	{
	  theCurrentItem = theCurrentItem->parent();
	}
      return theCurrentItem->text( 0 );
    }
  else
    return QString::null;
};

void SIG_ExperimentListView::selectItem( QString label )
{
  QListViewItem *theItem = currentItem();
  if( theItem )
    {
      while( theItem->parent() != 0)
	{
	  theItem = theItem->parent();
	}

      for(; theItem; theItem = theItem->itemBelow() )
	{
	  if( theItem->text( 0 ) == label )
	    {
	      setSelected( theItem, true );
	      break;
	    }
	}

      /* QListViewItemIterator it( theItem );
       * for( ; it.current(); it++ )
       * 	{
       *	  if( it.current()->text( 0 ) == label )
       *    {
       *	      setSelected( it.current(), true );
       *	      break;
       *	    }
       *	} */
    }
  else
    {
      QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
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
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowGPParameters();
      theExperiment->slotGPParameterImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};
  
void SIG_ExperimentListView::slotSimulationParametersImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowSimulationParameters();
      theExperiment->slotSimulationParameterImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotRobotImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowRobot();
      theExperiment->slotRobotImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotLanguageParametersImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowLanguageParameters();
      theExperiment->slotLanguageParameterImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotPopulationImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->slotPopulationImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotEnvironmentImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowEnvironment();
      theExperiment->slotEnvironmentImport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotGPParametersExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowGPParameters();
      theExperiment->slotGPParameterExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotSimulationParametersExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowSimulationParameters();
      theExperiment->slotSimulationParameterExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotLanguageParametersExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowLanguageParameters();
      theExperiment->slotLanguageParameterExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotPopulationExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->slotPopulationExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotEnvironmentExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowEnvironment();
      theExperiment->slotEnvironmentExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotGNUPlotExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      theExperiment->slotGNUPlotExport();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotAddIndividuals()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotAddIndividuals();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotDeleteIndividuals()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotDeleteIndividuals();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotResetIndividuals()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotResetPool();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotVisualizeIndividuals()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  // write all changes back to generate a new dynamechs terrain
  theExperiment->putAllIntoExperiment();

  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotVisualize();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotProgramExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotExportProgram();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotIndividualExport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotExportIndividual();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotProgramImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotImportProgram();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

void SIG_ExperimentListView::slotIndividualImport()
{
  SIG_Experiment *theExperiment = currentlySelectedExperiment();
  if( theExperiment )
    {
      slotShowIndividuals();
      theExperiment->allIndividualsView->slotImportIndividual();
    }
  else
    QMessageBox::information( this, "There is no experiment selected...", "Currently there is no experiment selected." );
};

} // close namespace

