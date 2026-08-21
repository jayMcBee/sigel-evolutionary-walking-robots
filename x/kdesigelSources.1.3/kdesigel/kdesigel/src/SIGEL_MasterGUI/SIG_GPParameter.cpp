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
#include <qfiledialog.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlcdnumber.h>
#include <qmessagebox.h>

#include "SIGEL_MasterGUI/SIG_GPParameter.h"
#include "SIGEL_MasterGUI/SIG_EditHostDialog.h"

#include "SIGEL_GP/SIG_GPPVMHost.h"

#include "SIGEL_Program/SIG_Program.h"

#include "SIGEL_Tools/SIG_IO.h" // take it out! Only DEBUG!

#ifndef _WINDOWS
#include <cstdlib>
#endif

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_GPParameter which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_GPParameter::SIG_GPParameter( QWidget* parent,  const char* name, WFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
#ifdef _WINDOWS
  : SIG_GPParameterBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( ::getenv( "SIGEL_ROOT" ) )
#else
  : SIG_GPParameterBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( std::getenv( "SIGEL_ROOT" ) )
#endif
{
  QObject::connect( listviewHosts,
		    SIGNAL( doubleClicked( QListViewItem * ) ),
		    SLOT( slotItemDoubleClicked( QListViewItem * ) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_GPParameter::~SIG_GPParameter()
{
    // no need to delete child widgets, Qt does it all for us
}

void SIG_GPParameter::slotChangeGraveyardDir()
{
  lineeditGraveyardDir->setText( QFileDialog::getExistingDirectory( "./", this, "getExistingDirDialogGraveyard", "Select Directory...", true ) );
};

void SIG_GPParameter::slotChangePoolImageDir()
{
  lineeditPoolImageDir->setText( QFileDialog::getExistingDirectory( "./", this, "getExistingDirDialogPoolImage", "Select Directory...", true ) );
};

void SIG_GPParameter::putIntoExperiment()
{
  theExperiment.gpParameter.setRandomSeed( spinboxRandomSeed->value() );
  theExperiment.population.getRandomizerPointer()->setNewSeed( spinboxRandomSeed->value() );
  theExperiment.gpParameter.setMinIndLength( spinboxMinIndLength->value() );
  if( checkboxIgnoreMaxIndLength->isChecked() )
    theExperiment.gpParameter.setMaxIndLength( 0 );
  else
    theExperiment.gpParameter.setMaxIndLength( spinboxMaxIndLength->value() );
  theExperiment.gpParameter.setResetEveryGeneration( spinboxResEvGen->value() );
  theExperiment.gpParameter.setMaxAge( spinboxMaxAge->value() );

  // put the probabilities into the experiment
  theExperiment.gpParameter.setXoverProb( sliderCrossover->value() );
  theExperiment.gpParameter.setMutationProb( sliderMutation->value() );
  theExperiment.gpParameter.setReproductionProb( 1000 - sliderMutation->value() - sliderCrossover->value() );

  // put the fitness function name into the experiment
  // theExperiment.gpParameter.setFitnessName( lineeditFitnessFunctionName->text() );
  switch( comboboxFitnessName->currentItem() )
    {
    case 0: // simple fitnessFunction
      theExperiment.gpParameter.setFitnessName("SimpleFitnessFunction");
      break;

    case 1:
      theExperiment.gpParameter.setFitnessName("RealSpeedFitnessFunction");
      break;

    case 2:
      theExperiment.gpParameter.setFitnessName("NiceWalkingFitnessFunction");
      break;

    case 3:
      theExperiment.gpParameter.setFitnessName("ZorcWalkingFitnessFunction");
      break;

    case 4:
      theExperiment.gpParameter.setFitnessName("StepperFitnessFunction");
      break;

    case 5:
      theExperiment.gpParameter.setFitnessName("RemoteZORCFitnessFunction");
      break;

    case 6:
    	theExperiment.gpParameter.setFitnessName("ForceFitnessFunction");
     	break;

		default:
			QMessageBox::information( 0, "Error !", "Internal Error -- fitness function is unknown to class SIG_GPParameter.");
    }

  // put the probabilities into the experiment
  theExperiment.gpParameter.setProbability( SIGEL_Program::ADD, sliderADD->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::CMP, sliderCMP->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::COPY, sliderCOPY->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::DELAY, sliderDELAY->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::DIV, sliderDIV->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::JMP, sliderJMP->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::LOAD, sliderLOAD->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::MAX, sliderMAX->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::MIN, sliderMIN->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::MOD, sliderMOD->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::MOVE, sliderMOVE->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::MUL, sliderMUL->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::NOP, sliderNOP->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::SENSE, sliderSENSE->value() );
  theExperiment.gpParameter.setProbability( SIGEL_Program::SUB, sliderSUB->value() );
  
  switch( comboboxTerminationBy->currentItem() )
    {
    case 0:
      theExperiment.gpParameter.setTerminationModel( SIGEL_GP::SIG_GPParameter::byUser );
      break;
    case 1:
      theExperiment.gpParameter.setTerminationModel( SIGEL_GP::SIG_GPParameter::byTime );
      break;
    case 2:
      theExperiment.gpParameter.setTerminationModel( SIGEL_GP::SIG_GPParameter::byGeneration );
      break;
    case 3:
      theExperiment.gpParameter.setTerminationModel( SIGEL_GP::SIG_GPParameter::byTimeGeneration );
      break;
    }

  // put the date into the experiment
  QDate theDate;
  theDate.setYMD( spinboxByTimeYear->value(), spinboxByTimeMonth->value(), spinboxByTimeDay->value());
  QTime theTime;
  theTime.setHMS( spinboxByTimeHour->value(), spinboxByTimeMins->value(), spinboxByTimeSecs->value() );
  QDateTime theDateTime( theDate, theTime );
  theExperiment.gpParameter.setTerminationTime( theDateTime );

  // put the duration into the experiment
  theExperiment.gpParameter.setTerminationDurationDays( spinboxByDurationDays->value() );
  theExperiment.gpParameter.setTerminationDurationHours( spinboxByDurationHours->value() );
  theExperiment.gpParameter.setTerminationDurationMinutes( spinboxByDurationMins->value() );
  theExperiment.gpParameter.setTerminationDurationSeconds( spinboxByDurationSecs->value() );

  // set whether the date should be used.
  if( radiobuttonInterpreteAsDate->isChecked() )
    theExperiment.gpParameter.setTerminationUsesDate( true );
  else
    theExperiment.gpParameter.setTerminationUsesDate( false );

  theExperiment.gpParameter.setTerminationGenerationNo( spinboxByGenerationNumber->value() );

  // set the tournaments per generation parameter
  double tournamentsPerGeneration = static_cast<double>( sliderTournamentsPerGeneration->value() ) / 1000;
  theExperiment.gpParameter.setTournamentsPerGeneration( tournamentsPerGeneration );

  // set the priority
  switch( comboboxPriority->currentItem() )
    {
    case 0:
      theExperiment.gpParameter.setPriority(SIGEL_GP::SIG_GPParameter::veryLow);
      break;
    case 1:
      theExperiment.gpParameter.setPriority(SIGEL_GP::SIG_GPParameter::low);
      break;
    case 2:
      theExperiment.gpParameter.setPriority(SIGEL_GP::SIG_GPParameter::normal);
      break;
    case 3:
      theExperiment.gpParameter.setPriority(SIGEL_GP::SIG_GPParameter::high);
      break;
    case 4:
      theExperiment.gpParameter.setPriority(SIGEL_GP::SIG_GPParameter::veryHigh);
      break;
    }

  // set live undead
  theExperiment.gpParameter.setLiveUndead( checkboxUseGraveyard->isChecked() );
  QDir graveyardDir( lineeditGraveyardDir->text() );
  theExperiment.gpParameter.setGraveYardDirectory( graveyardDir );

  // set poolimage directory options
  QDir poolImageDir( lineeditPoolImageDir->text() );
  theExperiment.gpParameter.setPoolImageDirectory( poolImageDir );
  if( checkboxUsePoolImage->isChecked() )
    theExperiment.gpParameter.setPoolImageGeneration( spinboxPoolImageFrequency->value() );
  else
    theExperiment.gpParameter.setPoolImageGeneration( 0 );

  // set the timeout
  theExperiment.gpParameter.setTimeOutMinutes( spinboxTimeout->value() );
};

void SIG_GPParameter::getOutOfExperiment()
{
  spinboxRandomSeed->setValue( theExperiment.gpParameter.getRandomSeed() );
  spinboxMinIndLength->setValue( theExperiment.gpParameter.getMinIndLength() );
  int maxLength = theExperiment.gpParameter.getMaxIndLength();
  if( maxLength == 0 )
    {
      checkboxIgnoreMaxIndLength->setChecked( true );
      spinboxMaxIndLength->setValue( theExperiment.gpParameter.getMinIndLength() );
    }
  else
    spinboxMaxIndLength->setValue( maxLength );
  spinboxResEvGen->setValue( theExperiment.gpParameter.getResetEveryGeneration() );
  spinboxMaxAge->setValue( theExperiment.gpParameter.getMaxAge() );
  
  // sliderReproduction->setValue( theExperiment.gpParameter.getReproductionProb() );
  sliderCrossover->setValue( theExperiment.gpParameter.getXoverProb() );
  sliderMutation->setValue( theExperiment.gpParameter.getMutationProb() );
  sliderCrossover->setValue( theExperiment.gpParameter.getXoverProb() );

  // get the fitness function name
  QString fitnessName = theExperiment.gpParameter.getFitnessName();
  if ( fitnessName == "SimpleFitnessFunction" )
    comboboxFitnessName->setCurrentItem(0);
  else if ( fitnessName == "RealSpeedFitnessFunction" )
      comboboxFitnessName->setCurrentItem(1);
  else if ( fitnessName == "NiceWalkingFitnessFunction" )
      comboboxFitnessName->setCurrentItem(2);
  else if ( fitnessName == "ZorcWalkingFitnessFunction" )
      comboboxFitnessName->setCurrentItem(3);
  else if ( fitnessName == "StepperFitnessFunction" )
      comboboxFitnessName->setCurrentItem(4);
  else if ( fitnessName == "RemoteZORCFitnessFunction" )
      comboboxFitnessName->setCurrentItem(5);
  else if ( fitnessName == "ForceFitnessFunction" )
  		comboboxFitnessName->setCurrentItem(6);
  else comboboxFitnessName->setCurrentItem(0);

  // get the probabilities
  sliderADD->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::ADD ) );
  sliderCMP->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::CMP ) );
  sliderCOPY->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::COPY ) );
  sliderDELAY->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::DELAY ) );
  sliderDIV->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::DIV ) );
  sliderJMP->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::JMP ) );
  sliderLOAD->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::LOAD ) );
  sliderMAX->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::MAX ) );
  sliderMIN->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::MIN ) );
  sliderMOD->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::MOD ) );
  sliderMOVE->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::MOVE ) );
  sliderMUL->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::MUL ) );
  sliderNOP->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::NOP ) );
  sliderSENSE->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::SENSE ) );
  sliderSUB->setValue( theExperiment.gpParameter.getProbability( SIGEL_Program::SUB ) );

  // set the termination model combo box
  switch( theExperiment.gpParameter.getTerminationModel() )
    {
    case SIGEL_GP::SIG_GPParameter::byUser:
      comboboxTerminationBy->setCurrentItem( 0 );
      break;
    case SIGEL_GP::SIG_GPParameter::byTime:
      comboboxTerminationBy->setCurrentItem( 1 );
      break;
    case SIGEL_GP::SIG_GPParameter::byGeneration:
      comboboxTerminationBy->setCurrentItem( 2 );
      break;
    case SIGEL_GP::SIG_GPParameter::byTimeGeneration:
      comboboxTerminationBy->setCurrentItem( 3 );
      break;
    }
  
  QDateTime theDateTime = theExperiment.gpParameter.getTerminationTime();
  spinboxByTimeDay->setValue( theDateTime.date().day() );
  spinboxByTimeMonth->setValue( theDateTime.date().month() );
  spinboxByTimeYear->setValue( theDateTime.date().year() );
  spinboxByTimeHour->setValue( theDateTime.time().hour() );
  spinboxByTimeMins->setValue( theDateTime.time().minute() );
  spinboxByTimeSecs->setValue( theDateTime.time().second() );

  // get the duration out of the experiment
  spinboxByDurationDays->setValue( theExperiment.gpParameter.getTerminationDurationDays() );
  spinboxByDurationHours->setValue( theExperiment.gpParameter.getTerminationDurationHours() );
  spinboxByDurationMins->setValue( theExperiment.gpParameter.getTerminationDurationMinutes() );
  spinboxByDurationSecs->setValue( theExperiment.gpParameter.getTerminationDurationSeconds() );

  // get whether the date should be used.
  if( theExperiment.gpParameter.getTerminationUsesDate() )
    radiobuttonInterpreteAsDate->setChecked( true );
  else
    radiobuttonInterpreteAsDuration->setChecked( true );

  spinboxByGenerationNumber->setValue( theExperiment.gpParameter.getTerminationGenerationNo() );

  // set the tournaments per generation parameter & display
  int tournamentsPerGeneration = static_cast<int>( theExperiment.gpParameter.getTournamentsPerGeneration() * 1000 );
  sliderTournamentsPerGeneration->setValue( tournamentsPerGeneration );

  // display the REAL value, according to the calculations in SIG_GPManager.cpp, also see slotTourPerGen()
  double tpg = ((double)sliderTournamentsPerGeneration->value() / 1000.0) * (double)theExperiment.population.getSize();
  lcdnumberTournamentsPerGeneration->display( static_cast<int>( tpg ) );

  // get the priority
  switch( theExperiment.gpParameter.getPriority() )
    {
    case SIGEL_GP::SIG_GPParameter::veryLow:
      comboboxPriority->setCurrentItem( 0 );
      break;
    case SIGEL_GP::SIG_GPParameter::low:
      comboboxPriority->setCurrentItem( 1 );
      break;
    case SIGEL_GP::SIG_GPParameter::normal:
      comboboxPriority->setCurrentItem( 2 );
      break;
    case SIGEL_GP::SIG_GPParameter::high:
      comboboxPriority->setCurrentItem( 3 );
      break;
    case SIGEL_GP::SIG_GPParameter::veryHigh:
      comboboxPriority->setCurrentItem( 4 );
      break;
    }
  
  // set the graveyard stuff widgets
  if( theExperiment.gpParameter.getLiveUndead() )
    checkboxUseGraveyard->setChecked( true );
  else
    checkboxUseGraveyard->setChecked( false );
  QDir graveyardDir = theExperiment.gpParameter.getGraveYardDirectory();
  lineeditGraveyardDir->setText( graveyardDir.absPath() );

  QDir poolImageDir = theExperiment.gpParameter.getPoolImageDirectory();
  lineeditPoolImageDir->setText( poolImageDir.absPath() );
  int poolImageGeneration = theExperiment.gpParameter.getPoolImageGeneration();
  if (poolImageGeneration == 0)
    checkboxUsePoolImage->setChecked( false );
  else
    {
      checkboxUsePoolImage->setChecked( true );
      spinboxPoolImageFrequency->setValue( poolImageGeneration );
    }
  
  // enter the PVM hosts into the list...
  // clear list
  listviewHosts->clear();
  QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();

  QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );

  for ( ; it.current(); ++it )
  {
      // create a new listview item for each found host
      QListViewItem *newItem = new QListViewItem( listviewHosts );
      // set the pixmap
      if( it.current()->enabled )
	newItem->setPixmap(0, QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) );
      else
	newItem->setPixmap(0, QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) );
      // set the hostname
      newItem->setText( 1, it.current()->name );
      // set maximal slaves
      newItem->setText( 2, QString::number( it.current()->maxSlaves ) );
#ifdef _WINDOWS
      newItem->setText( 3, it.current()->executableDir.path() );
#else
      newItem->setText( 3, it.current()->executableDir.absPath() );
#endif
    }

  // set the timeout
  spinboxTimeout->setValue( theExperiment.gpParameter.getTimeOutMinutes() );
};

void SIG_GPParameter::slotAddHost()
{
  // create an show the dialog
  SIG_EditHostDialog editDialog( 0, "editDialogAddHosts", true, 0 );
  editDialog.checkboxEnableHost->setChecked( true );
  editDialog.lineeditHostName->setFocus();
  editDialog.lineeditSlaveDirectory->setText( QDir::currentDirPath() );
  editDialog.setCaption( "Add host..." );
  switch ( editDialog.exec() )
    {
      // the OK button was pressed
    case QDialog::Accepted:
      // lets first see if there is no host under that name...
      QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();
      QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );
      bool isThere = false;
      for ( ; it.current(); ++it )
	{
	  if( it.current()->name == editDialog.lineeditHostName->text() )
	    {
	      isThere = true;
	      break;
	    }
        }
      if( !isThere && !editDialog.lineeditHostName->text().isEmpty() )
	{
	  QString newName = editDialog.lineeditHostName->text();
	  int newMaxSlaves = editDialog.spinboxMaximalNumberOfProcesses->value();
	  bool newEnabled = editDialog.checkboxEnableHost->isChecked();
	  QString newSlaveDirectory = editDialog.lineeditSlaveDirectory->text();

	  // has to changed!!! has to be changed!!! QDir has to be set right!!!
	  SIGEL_GP::SIG_GPPVMHost *newHost = new SIGEL_GP::SIG_GPPVMHost( newName, newMaxSlaves, newEnabled, QDir( newSlaveDirectory ) );

	  QListViewItem *newListViewItem = new QListViewItem( listviewHosts );
	  // set enabled
	  if ( newEnabled )
	    newListViewItem->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) );
	  else
	    newListViewItem->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) );
	  // set Hostname
	  newListViewItem->setText( 1, newName );
	  //set maximal slaves
	  newListViewItem->setText( 2, QString::number( newMaxSlaves ) );
	  newListViewItem->setText( 3, newSlaveDirectory );
	  hostList.append( newHost );
	}
      else
	{
	  QMessageBox::information( 0, "Error...", "The host " + editDialog.lineeditHostName->text() + " is either already present or you entered no name!" );
	}
      break;
    }
};

void SIG_GPParameter::slotEditHost()
{
  slotItemDoubleClicked( listviewHosts->currentItem() );
};

void SIG_GPParameter::slotDeleteHost()
{
  /*
   * this is the list of SIG_GPPVMHosts which have to be deleted. we don't
   * delete the host directly as the iterator would get confused.
   *
   * NOTE: the 2003 comment here said setAutoDelete was not true on the host
   * list. It was (SIG_GPParameter.cpp, constructor), so remove() below was
   * the delete. The flag is gone and the delete is now written out.
   */
  QList<SIGEL_GP::SIG_GPPVMHost> deleteListHosts;
  QList<QListViewItem> deleteListViewItems;
  
  // iterate over the list items
  QListViewItemIterator listIt( listviewHosts );
  for( ; listIt.current(); listIt++ )
    // is the current one selected?
    if( listIt.current()->isSelected() )
      {
	// first enter the item into the list of items to delete
	deleteListViewItems.append( listIt.current() );

	// get the name of the host to find it in the host list
	QString currentName = listIt.current()->text( 1 );
	// SIGEL_GP::SIG_GPPVMHost *theHost = 0; i think i don't need this anymore
	QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();
	QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );
	// iterate over the host list to find the SIG_GPPVMHost object belonging to host currentName
	for ( ; it.current(); ++it )
	  {
	    if( it.current()->name == currentName )
	      {
		deleteListHosts.append( it.current() );
		break;
	      }
	  }
	/* if( theHost )
	 * hostList.remove( theHost );
	 * listviewHosts->takeItem( listIt.current() );
	 */
      }
  
// now lets break the shit up!
  QList<SIGEL_GP::SIG_GPPVMHost> &hostList2 = theExperiment.gpParameter.getHostList();
  QListIterator<SIGEL_GP::SIG_GPPVMHost> hostIt( deleteListHosts );
  for( ; hostIt.current(); ++hostIt )
    {
      // deleteListHosts can hold the same pointer twice: the search above
      // matches on name, and nothing forbids two hosts with one name. Only
      // the removal that actually unlinked may free.
      SIGEL_GP::SIG_GPPVMHost *host = hostIt.current();
      if ( hostList2.remove( host ) )
	delete host;
    }
  QListIterator<QListViewItem> itemIt( deleteListViewItems );
  for( ; itemIt.current(); ++itemIt )
    listviewHosts->takeItem( itemIt.current() );
  
};

void SIG_GPParameter::slotEnableAllHosts()
{
  QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();
  QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );
  for ( ; it.current(); ++it )
    {
      it.current()->enabled = true;
    }
  QListViewItemIterator listIt( listviewHosts );
  for ( ; listIt.current(); ++listIt )
    {
      listIt.current()->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) );
    }
};

void SIG_GPParameter::slotDisableAllHosts()
{
  QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();
  QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );
  for ( ; it.current(); ++it )
    {
      it.current()->enabled = false;
    }
  QListViewItemIterator listIt( listviewHosts );
  for ( ; listIt.current(); ++listIt )
    {
      listIt.current()->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) );
    }
};

void SIG_GPParameter::slotItemDoubleClicked( QListViewItem * theItem )
{
  if( theItem )
    {
      // count how many host items are selected, so we can display the dialog the right way
      int numberOfSelectedHosts = 0;
      QListViewItemIterator itemIt( listviewHosts );
      for( ; itemIt.current(); ++itemIt )
	{
	  if( itemIt.current()->isSelected() )
	    ++numberOfSelectedHosts;
	}
      
      QString currentName = theItem->text( 1 );
      SIGEL_GP::SIG_GPPVMHost *theHost = 0;
      QList<SIGEL_GP::SIG_GPPVMHost> &hostList = theExperiment.gpParameter.getHostList();
      QListIterator<SIGEL_GP::SIG_GPPVMHost> it( hostList );
      for ( ; it.current(); ++it )
	{
	  if( it.current()->name == currentName )
	    {
	      // we found the host, so save it in the pointer and quit
	      theHost = it.current();
	      break;
	    }
	}
      
      SIG_EditHostDialog editDialog( 0, "editDialogEditHosts", true, 0 );
      if( numberOfSelectedHosts == 1 )
	{
	  editDialog.lineeditHostName->setText( theHost->name );
	  editDialog.spinboxMaximalNumberOfProcesses->setValue( theHost->maxSlaves );
	  editDialog.lineeditSlaveDirectory->setText( theHost->executableDir.absPath() );
	  if( theHost->enabled )
	    editDialog.checkboxEnableHost->setChecked( true );
	  else
	    editDialog.checkboxEnableHost->setChecked( false );
	  editDialog.lineeditHostName->setFocus();
	  editDialog.setCaption( "Edit host " + theHost->name + "..." );
	}
      else
	{
	  editDialog.lineeditHostName->hide();
	  editDialog.lineeditSlaveDirectory->setText( theHost->executableDir.absPath() );
	  editDialog.spinboxMaximalNumberOfProcesses->setValue( theHost->maxSlaves );
	  
	  if( theHost->enabled )
	    editDialog.checkboxEnableHost->setChecked( true );
	  else
	    editDialog.checkboxEnableHost->setChecked( false );
	  editDialog.setCaption( "Edit hosts..." );
	  editDialog.resize( QSize() );
	}
      switch( editDialog.exec() )
	{
	case QDialog::Accepted:
	  /*
	   * warning! it is important that if one item was selected the hostname can be changed
	   * but must NOT be changed into an existing name oder empty name!!!
	   */

	  if( numberOfSelectedHosts == 1 )
	    {
	      QString newName = editDialog.lineeditHostName->text();
	      bool isThere = false;
	      if( newName != currentName )
		{
		  QListIterator<SIGEL_GP::SIG_GPPVMHost> it2( hostList );
		  for ( ; it2.current(); ++it2 )
		    {
		      if( it2.current()->name == newName )
			{
			  isThere = true;
			  break;
			}
		    }
		}
	      if( !isThere && !newName.isEmpty() )
		{
		  int newMaxSlaves = editDialog.spinboxMaximalNumberOfProcesses->value();
		  bool newEnabled = editDialog.checkboxEnableHost->isChecked();
		  QString newSlaveDirectory = editDialog.lineeditSlaveDirectory->text();
		  theHost->name = newName;
		  theHost->maxSlaves = newMaxSlaves;
		  theHost->enabled = newEnabled;
		  theHost->executableDir = QDir( newSlaveDirectory );
		  if ( newEnabled )
		    theItem->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) );
		  else
		    theItem->setPixmap( 0, QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) );
		  // set Hostname
		  theItem->setText( 1, newName );
		  //set maximal slaves
		  theItem->setText( 2, QString::number( newMaxSlaves ) );
		  theItem->setText( 3, newSlaveDirectory );
		}
	      else
		{
		  QMessageBox::information( 0, "Error...", "The host " + editDialog.lineeditHostName->text() + " is either already present or you entered no name!" );
		}
	    }
	  else
	    {
	      int newMaxSlaves = editDialog.spinboxMaximalNumberOfProcesses->value();
	      bool newEnabled = editDialog.checkboxEnableHost->isChecked();
	      QString newSlaveDirectory = editDialog.lineeditSlaveDirectory->text();
	      QListViewItemIterator itemIt2( listviewHosts );
	      for( ; itemIt2.current(); ++itemIt2 )
		{
		  if( itemIt2.current()->isSelected() )
		    {
		      QString currentHostName = itemIt2.current()->text( 1 );
		      QListIterator<SIGEL_GP::SIG_GPPVMHost> hostIt( hostList );
		      for ( ; hostIt.current(); ++hostIt )
			{
			  if( hostIt.current()->name == currentHostName )
			    {
			      // set all the stuff according to the dialog
			      hostIt.current()->maxSlaves = newMaxSlaves;
			      hostIt.current()->enabled = newEnabled;
			      hostIt.current()->executableDir = QDir( newSlaveDirectory );
			    }
			} // for over all

		      // set the listviewItems right...
		      if( newEnabled)
			itemIt2.current()->setPixmap(0, QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) );
		      else
			itemIt2.current()->setPixmap(0, QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) );
		      itemIt2.current()->setText(2, QString::number( newMaxSlaves ) );
		      itemIt2.current()->setText(3, newSlaveDirectory );
		    } // if( itemIt2.current()->isSelected )
		}
	    }
	  break;
	} // end of switch statement
    } // end of if( theItem )
};

void SIG_GPParameter::slotMutationChanged( int newMutationValue )
{
  if( newMutationValue + sliderCrossover->value() > 1000 )
    sliderMutation->setValue( 1000 - sliderCrossover->value() );
  double mutationValue = static_cast<double>( sliderMutation->value() );
  double crossoverValue = static_cast<double>( sliderCrossover->value() );
  lcdnumberMutation->display( mutationValue / 10 );
  sliderReproduction->setValue( 1000 - sliderMutation->value() - sliderCrossover->value() );
  lcdnumberReproduction->display( ( 1000 - mutationValue - crossoverValue) / 10 );
}

void SIG_GPParameter::slotCrossoverChanged( int newCrossoverValue )
{
  if( newCrossoverValue + sliderMutation->value() > 1000 )
    sliderCrossover->setValue( 1000 - sliderMutation->value() );
  double mutationValue = static_cast<double>( sliderMutation->value() );
  double crossoverValue = static_cast<double>( sliderCrossover->value() );
  lcdnumberCrossover->display( crossoverValue / 10 );
  sliderReproduction->setValue( 1000 - sliderMutation->value() - sliderCrossover->value() );
  lcdnumberReproduction->display( (1000 - crossoverValue - mutationValue) / 10 );
}

void SIG_GPParameter::slotTourPerGenChanged( int newParmTPG )
{  double   tpg;

   // display the *Real* value used in SIG_GPManager later on to call
   // CreateTours() and thus the *actual* number of Tournament per generation
   tpg = ((double)sliderTournamentsPerGeneration->value() / 1000.0) * (double)theExperiment.population.getSize();
   lcdnumberTournamentsPerGeneration->display( static_cast<int>( tpg ) );
}

}
