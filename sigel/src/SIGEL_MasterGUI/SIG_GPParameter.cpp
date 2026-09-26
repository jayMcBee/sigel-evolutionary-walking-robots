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
#include <QLineEdit>
#include <QTimer>
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

#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"
#include "SIGEL_GP/SIG_GPPVMHost.h"

#include "SIGEL_Program/SIG_Program.h"

#include "SIGEL_Tools/SIG_IO.h"

#ifndef _WINDOWS
#include <cstdlib>
#endif

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_GPParameter which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_GPParameter::SIG_GPParameter( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
#ifdef _WINDOWS
  : SIG_GPParameterBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( ::getenv( "SIGEL_ROOT" ) )
#else
  : SIG_GPParameterBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( std::getenv( "SIGEL_ROOT" ) )
#endif
{
  for ( const SIGEL_GP::SIG_GPFitnessFunction *fitnessFunction : SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions() )
    fitnessFunctionList->addItem( fitnessFunction->name() );

  // A note, not a field label: drawn in the theme's secondary text colour, and
  // two lines high, so the page does not move when a description wraps.
  fitnessFunctionDescription->setForegroundRole( QPalette::PlaceholderText );
  fitnessFunctionDescription->setMinimumHeight( 2 * fitnessFunctionDescription->fontMetrics().lineSpacing() );
  QObject::connect( fitnessFunctionList, &QComboBox::currentIndexChanged, this, [this]()
    {
      const std::optional<int> index = selectedFitnessFunction();
      fitnessFunctionDescription->setText( index ? SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions()[*index]->description() : QString() );
    } );

  QObject::connect( listviewHosts,
		    SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
		    SLOT( slotItemDoubleClicked( QTreeWidgetItem * ) ) );

  // Each instruction's slider drives its counter. These are connected here,
  // not in the .ui: QLCDNumber::display is overloaded, and some uic versions
  // write the connection without choosing an overload, which does not compile.
  const QList<QPair<QSlider *, QLCDNumber *>> instructionCounters = {
      { sliderADD, lcdnumberADD },
      { sliderCMP, lcdnumberCMP },
      { sliderCOPY, lcdnumberCOPY },
      { sliderDELAY, lcdnumberDELAY },
      { sliderDIV, lcdnumberDIV },
      { sliderJMP, lcdnumberJMP },
      { sliderLOAD, lcdnumberLOAD },
      { sliderMAX, lcdnumberMAX },
      { sliderMIN, lcdnumberMIN },
      { sliderMOD, lcdnumberMOD },
      { sliderMOVE, lcdnumberMOVE },
      { sliderMUL, lcdnumberMUL },
      { sliderNOP, lcdnumberNOP },
      { sliderSENSE, lcdnumberSENSE },
      { sliderSUB, lcdnumberSUB }
    };
  for ( const auto &[slider, counter] : instructionCounters )
    QObject::connect( slider, &QSlider::valueChanged,
		      counter, qOverload<int>( &QLCDNumber::display ) );

  // The counter has four digits, and on overflow it keeps showing the old ones.
  QObject::connect( lcdnumberTournamentsPerGeneration, &QLCDNumber::overflow, this, [this]()
    {
      SIGEL_Tools::SIG_IO::cerr << "The tournaments per generation counter shows "
				<< lcdnumberTournamentsPerGeneration->digitCount()
				<< " digits and cannot display "
				<< lcdnumberTournamentsPerGeneration->intValue() << "." << Qt::endl;
    } );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_GPParameter::~SIG_GPParameter()
{
    // no need to delete child widgets, Qt does it all for us
}

std::optional<int> SIG_GPParameter::selectedFitnessFunction() const
{
  // -1 is Qt's "nothing selected"
  const int index = fitnessFunctionList->currentIndex();
  if ( index == -1 )
    return std::nullopt;
  return index;
}

void SIG_GPParameter::slotChangeGraveyardDir()
{
  lineeditGraveyardDir->setText( QFileDialog::getExistingDirectory( this, "Select Directory", "./" ) );
};

void SIG_GPParameter::slotChangePoolImageDir()
{
  lineeditPoolImageDir->setText( QFileDialog::getExistingDirectory( this, "Select Directory", "./" ) );
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

  // with nothing selected, the stored ID stays
  if ( const std::optional<int> index = selectedFitnessFunction() )
    theExperiment.gpParameter.setFitnessName( SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions()[*index]->serializedId() );

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
  
  switch( comboboxTerminationBy->currentIndex() )
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
  theDate.setDate( spinboxByTimeYear->value(), spinboxByTimeMonth->value(), spinboxByTimeDay->value());
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
  switch( comboboxPriority->currentIndex() )
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

  // an unknown ID selects nothing; the text is set here because selecting the
  // index already shown emits no signal
  const QString fitnessName = theExperiment.gpParameter.getFitnessName();
  const std::optional<int> fitnessIndex = SIGEL_GP::SIG_GPFitnessFunctionRegistry::indexOf( fitnessName );
  fitnessFunctionList->setCurrentIndex( fitnessIndex ? *fitnessIndex : -1 );
  fitnessFunctionDescription->setText( fitnessIndex
    ? SIGEL_GP::SIG_GPFitnessFunctionRegistry::fitnessFunctions()[*fitnessIndex]->description()
    : "The experiment names the fitness function \"" + fitnessName + "\", which this build does not have." );

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
      comboboxTerminationBy->setCurrentIndex( 0 );
      break;
    case SIGEL_GP::SIG_GPParameter::byTime:
      comboboxTerminationBy->setCurrentIndex( 1 );
      break;
    case SIGEL_GP::SIG_GPParameter::byGeneration:
      comboboxTerminationBy->setCurrentIndex( 2 );
      break;
    case SIGEL_GP::SIG_GPParameter::byTimeGeneration:
      comboboxTerminationBy->setCurrentIndex( 3 );
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
      comboboxPriority->setCurrentIndex( 0 );
      break;
    case SIGEL_GP::SIG_GPParameter::low:
      comboboxPriority->setCurrentIndex( 1 );
      break;
    case SIGEL_GP::SIG_GPParameter::normal:
      comboboxPriority->setCurrentIndex( 2 );
      break;
    case SIGEL_GP::SIG_GPParameter::high:
      comboboxPriority->setCurrentIndex( 3 );
      break;
    case SIGEL_GP::SIG_GPParameter::veryHigh:
      comboboxPriority->setCurrentIndex( 4 );
      break;
    }
  
  // set the graveyard stuff widgets
  if( theExperiment.gpParameter.getLiveUndead() )
    checkboxUseGraveyard->setChecked( true );
  else
    checkboxUseGraveyard->setChecked( false );
  QDir graveyardDir = theExperiment.gpParameter.getGraveYardDirectory();
  lineeditGraveyardDir->setText( graveyardDir.absolutePath() );

  QDir poolImageDir = theExperiment.gpParameter.getPoolImageDirectory();
  lineeditPoolImageDir->setText( poolImageDir.absolutePath() );
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
  QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();

  

  for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
  {
      // create a new listview item for each found host
      QTreeWidgetItem *newItem = new QTreeWidgetItem();
      listviewHosts->insertTopLevelItem( 0, newItem );
      // set the pixmap
      if( it->enabled )
	newItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) ) );
      else
	newItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) ) );
      // set the hostname
      newItem->setText( 1, it->name );
      // set maximal slaves
      newItem->setText( 2, QString::number( it->maxSlaves ) );
#ifdef _WINDOWS
      newItem->setText( 3, it->executableDir.path() );
#else
      newItem->setText( 3, it->executableDir.absolutePath() );
#endif
    }

  // set the timeout
  spinboxTimeout->setValue( theExperiment.gpParameter.getTimeOutMinutes() );
};

void SIG_GPParameter::slotAddHost()
{
  // create an show the dialog
  SIG_EditHostDialog editDialog( this, "editDialogAddHosts", true, Qt::WindowFlags() );
  editDialog.checkboxEnableHost->setChecked( true );
  editDialog.lineeditHostName->setFocus();
  editDialog.lineeditSlaveDirectory->setText( QDir::currentPath() );
  editDialog.setWindowTitle( "Add Host" );
  switch ( editDialog.exec() )
    {
      // the OK button was pressed
    case QDialog::Accepted:
      // lets first see if there is no host under that name...
      QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();
      bool isThere = false;
      for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
	{
	  if( it->name == editDialog.lineeditHostName->text() )
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

	  QTreeWidgetItem *newListViewItem = new QTreeWidgetItem();

	  listviewHosts->insertTopLevelItem( 0, newListViewItem );
	  // set enabled
	  if ( newEnabled )
	    newListViewItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) ) );
	  else
	    newListViewItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) ) );
	  // set Hostname
	  newListViewItem->setText( 1, newName );
	  //set maximal slaves
	  newListViewItem->setText( 2, QString::number( newMaxSlaves ) );
	  newListViewItem->setText( 3, newSlaveDirectory );
	  hostList.append( newHost );
	}
      else
	{
	  QMessageBox::information( this, "Error", "The host " + editDialog.lineeditHostName->text() + " is either already present or you entered no name." );
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
   * the delete. The delete is written out below.
   */
  QList<SIGEL_GP::SIG_GPPVMHost *> deleteListHosts;
  QList<QTreeWidgetItem *> deleteListViewItems;
  
  // iterate over the list items
  QTreeWidgetItemIterator listIt( listviewHosts );
  for( ; *listIt; listIt++ )
    // is the current one selected?
    if( (*listIt)->isSelected() )
      {
	// first enter the item into the list of items to delete
	deleteListViewItems.append( *listIt );

	// get the name of the host to find it in the host list
	QString currentName = (*listIt)->text( 1 );
	// SIGEL_GP::SIG_GPPVMHost *theHost = 0; i think i don't need this anymore
	QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();
	
	// iterate over the host list to find the SIG_GPPVMHost object belonging to host currentName
	for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
	  {
	    if( it->name == currentName )
	      {
		deleteListHosts.append( it );
		break;
	      }
	  }
	/* if( theHost )
	 * hostList.remove( theHost );
	 * listviewHosts->takeTopLevelItem( listviewHosts->indexOfTopLevelItem( listIt ) );
	 */
      }
  
// now lets break the shit up!
  QList<SIGEL_GP::SIG_GPPVMHost *> &hostList2 = theExperiment.gpParameter.getHostList();
  
  for ( SIGEL_GP::SIG_GPPVMHost *hostIt : deleteListHosts )
    {
      // deleteListHosts can hold the same pointer twice: the search above
      // matches on name and breaks at the first hit, and nothing forbids two
      // hosts with one name. Only the removal that actually unlinked may free.
      //
      // The same 2003 defect also orphans the second host of such a pair: both
      // rows leave the list view, but only the first host leaves hostList, so
      // the trainer still spawns on it and writeToFile still persists it.
      // Left as it was -- fixing it means changing what the GUI does.
      SIGEL_GP::SIG_GPPVMHost *host = hostIt;
      if ( hostList2.removeOne( host ) )
	delete host;
    }
  
  for ( QTreeWidgetItem *itemIt : deleteListViewItems )
    listviewHosts->takeTopLevelItem( listviewHosts->indexOfTopLevelItem( itemIt ) );
  
};

void SIG_GPParameter::slotEnableAllHosts()
{
  QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();
  
  for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
    {
      it->enabled = true;
    }
  QTreeWidgetItemIterator listIt( listviewHosts );
  for ( ; *listIt; ++listIt )
    {
      (*listIt)->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) ) );
    }
};

void SIG_GPParameter::slotDisableAllHosts()
{
  QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();
  
  for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
    {
      it->enabled = false;
    }
  QTreeWidgetItemIterator listIt( listviewHosts );
  for ( ; *listIt; ++listIt )
    {
      (*listIt)->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) ) );
    }
};

void SIG_GPParameter::slotItemDoubleClicked( QTreeWidgetItem * theItem )
{
  if( theItem )
    {
      // count how many host items are selected, so we can display the dialog the right way
      int numberOfSelectedHosts = 0;
      QTreeWidgetItemIterator itemIt( listviewHosts );
      for ( ; *itemIt; ++itemIt )
	{
	  if( (*itemIt)->isSelected() )
	    ++numberOfSelectedHosts;
	}
      
      QString currentName = theItem->text( 1 );
      SIGEL_GP::SIG_GPPVMHost *theHost = 0;
      QList<SIGEL_GP::SIG_GPPVMHost *> &hostList = theExperiment.gpParameter.getHostList();
      
      for ( SIGEL_GP::SIG_GPPVMHost *it : hostList )
	{
	  if( it->name == currentName )
	    {
	      // we found the host, so save it in the pointer and quit
	      theHost = it;
	      break;
	    }
	}
      
      SIG_EditHostDialog editDialog( this, "editDialogEditHosts", true, Qt::WindowFlags() );
      if( numberOfSelectedHosts == 1 )
	{
	  editDialog.lineeditHostName->setText( theHost->name );
	  editDialog.spinboxMaximalNumberOfProcesses->setValue( theHost->maxSlaves );
	  editDialog.lineeditSlaveDirectory->setText( theHost->executableDir.absolutePath() );
	  if( theHost->enabled )
	    editDialog.checkboxEnableHost->setChecked( true );
	  else
	    editDialog.checkboxEnableHost->setChecked( false );
	  editDialog.lineeditHostName->setFocus();
	  // Qt 6 selects a line edit's text when a dialog gives it focus and
	  // Qt 2 did not, so a typed character REPLACES the pre-filled host
	  // name here where 1.3 appends to it.
	  // See SIG_LanguageParameters.cpp for the full note; slotAddHost has
	  // the same setFocus() and does NOT need this, because the field it
	  // focuses is empty there.
	  { QLineEdit *le = editDialog.lineeditHostName; QTimer::singleShot( 0, le, [le]{ le->end( false ); } ); }
	  editDialog.setWindowTitle( "Edit Host \"" + theHost->name + "\"" );
	}
      else
	{
	  editDialog.lineeditHostName->hide();
	  editDialog.lineeditSlaveDirectory->setText( theHost->executableDir.absolutePath() );
	  editDialog.spinboxMaximalNumberOfProcesses->setValue( theHost->maxSlaves );
	  
	  if( theHost->enabled )
	    editDialog.checkboxEnableHost->setChecked( true );
	  else
	    editDialog.checkboxEnableHost->setChecked( false );
	  editDialog.setWindowTitle( "Edit Hosts" );
	  editDialog.resize( QSize() );
	}
      switch( editDialog.exec() )
	{
	case QDialog::Accepted:
	  /*
	   * warning! it is important that if one item was selected the hostname can be changed
	   * but must NOT be changed into an existing name or empty name!!!
	   */

	  if( numberOfSelectedHosts == 1 )
	    {
	      QString newName = editDialog.lineeditHostName->text();
	      bool isThere = false;
	      if( newName != currentName )
		{
		  
		  for ( SIGEL_GP::SIG_GPPVMHost *it2 : hostList )
		    {
		      if( it2->name == newName )
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
		    theItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) ) );
		  else
		    theItem->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) ) );
		  // set Hostname
		  theItem->setText( 1, newName );
		  //set maximal slaves
		  theItem->setText( 2, QString::number( newMaxSlaves ) );
		  theItem->setText( 3, newSlaveDirectory );
		}
	      else
		{
		  QMessageBox::information( this, "Error", "The host " + editDialog.lineeditHostName->text() + " is either already present or you entered no name." );
		}
	    }
	  else
	    {
	      int newMaxSlaves = editDialog.spinboxMaximalNumberOfProcesses->value();
	      bool newEnabled = editDialog.checkboxEnableHost->isChecked();
	      QString newSlaveDirectory = editDialog.lineeditSlaveDirectory->text();
	      QTreeWidgetItemIterator itemIt2( listviewHosts );
	      for( ; *itemIt2; ++itemIt2 )
		{
		  if( (*itemIt2)->isSelected() )
		    {
		      QString currentHostName = (*itemIt2)->text( 1 );
		      for ( SIGEL_GP::SIG_GPPVMHost *hostIt : hostList )
			{
			  if( hostIt->name == currentHostName )
			    {
			      // set all the stuff according to the dialog
			      hostIt->maxSlaves = newMaxSlaves;
			      hostIt->enabled = newEnabled;
			      hostIt->executableDir = QDir( newSlaveDirectory );
			    }
			} // for over all

		      // set the listviewItems right...
		      if( newEnabled)
			(*itemIt2)->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/allow.xpm" ) ) );
		      else
			(*itemIt2)->setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/disallow.xpm" ) ) );
		      (*itemIt2)->setText(2, QString::number( newMaxSlaves ) );
		      (*itemIt2)->setText(3, newSlaveDirectory );
		    } // if( (*itemIt2)->isSelected )
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
