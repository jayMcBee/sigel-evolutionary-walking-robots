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
// #include <qpixmap.h>
#include <QTreeWidget>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include "SIGEL_MasterGUI/SIG_LanguageParameters.h"
#include "SIGEL_MasterGUI/SIG_EditCommandDialog.h"

#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Robot/SIG_CommandParameters.h"

#include "SIGEL_Tools/SIG_IO.h"

#include <cstdlib>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_LanguageParameters which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_LanguageParameters::SIG_LanguageParameters( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
#ifdef _WINDOWS
  : SIG_LanguageParametersBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( ::getenv( "SIGEL_ROOT" ) )//, allow( sigelRoot + "/pixmaps/allow.xpm" ), disallow( sigelRoot + "/pixmaps/disallow.xpm" )
#else
  : SIG_LanguageParametersBase( parent, name, fl ), theExperiment( theExperiment ), sigelRoot( std::getenv( "SIGEL_ROOT" ) )//, allow( sigelRoot + "/pixmaps/allow.xpm" ), disallow( sigelRoot + "/pixmaps/disallow.xpm" )
#endif
{
  // sigelRoot = QString( std::getenv( "SIGEL_ROOT") );
  allow = QPixmap( sigelRoot + "/pixmaps/allow.xpm" );
  disallow = QPixmap( sigelRoot + "/pixmaps/disallow.xpm" );
  // get the language parameters out of the experiment
  SIGEL_Robot::SIG_LanguageParameters *languageParameters = theExperiment.robot.getLangParam();

  // update the list
  QTreeWidgetItem *newItem;

  if( languageParameters->hasCommand( "SUB" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "SUB" );
      QString duration = QString::number( languageParameters->getCommand( "SUB" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "SENSE" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "SENSE" );
      QString duration = QString::number( languageParameters->getCommand( "SENSE" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "NOP" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "NOP" );
      QString duration = QString::number( languageParameters->getCommand( "NOP" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "MUL" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "MUL" );
      QString duration = QString::number( languageParameters->getCommand( "MUL" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "MOVE" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "MOVE" );
      QString duration = QString::number( languageParameters->getCommand( "MOVE" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "MOD" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "MOD" );
      QString duration = QString::number( languageParameters->getCommand( "MOD" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "MIN" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "MIN" );
      QString duration = QString::number( languageParameters->getCommand( "MIN" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "MAX" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "MAX" );
      QString duration = QString::number( languageParameters->getCommand( "MAX" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  if( languageParameters->hasCommand( "LOAD" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "LOAD" );
      QString duration = QString::number( languageParameters->getCommand( "LOAD" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "JMP" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "JMP" );
      QString duration = QString::number( languageParameters->getCommand( "JMP" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "DIV" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "DIV" );
      QString duration = QString::number( languageParameters->getCommand( "DIV" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "DELAY" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "DELAY" );
      QString duration = QString::number( languageParameters->getCommand( "DELAY" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "COPY" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "COPY" );
      QString duration = QString::number( languageParameters->getCommand( "COPY" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "CMP" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "CMP" );
      QString duration = QString::number( languageParameters->getCommand( "CMP" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }

  if( languageParameters->hasCommand( "ADD" ) )
    {
      newItem  = new QTreeWidgetItem();
      listviewCommands->insertTopLevelItem( 0, newItem );
      newItem->setIcon( 0, QIcon( allow ) );
      newItem->setText( 1, "ADD" );
      QString duration = QString::number( languageParameters->getCommand( "ADD" )->getDuration(), 'g', 10 );
      newItem->setText( 2, duration );
    }
  
  spinboxNumberOfRegisters->setValue( languageParameters->getMemorySize() );
  spinboxRegisterWidth->setValue( languageParameters->getRegisterWidth() );

  // editCommandDialog = new SIG_EditCommandDialog( this, "EditCommandDialog", true );

  QObject::connect( listviewCommands,
		    SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
		    this,
		    SLOT( slotCommandDoubleClicked( QTreeWidgetItem *) ) );
};

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_LanguageParameters::~SIG_LanguageParameters()
{
    // no need to delete child widgets, Qt does it all for us
};

void SIG_LanguageParameters::putIntoExperiment()
{
  // we don't need to update the command as they are updated when one works on them

  theExperiment.robot.getLangParam()->setMemorySize( spinboxNumberOfRegisters->value() );
  theExperiment.robot.getLangParam()->setRegisterWidth( spinboxRegisterWidth->value() );
  theExperiment.robot.getLangParam()->setMaximalDelayTime( spinboxMaximalDelayTime->value() );
};

void SIG_LanguageParameters::getOutOfExperiment()
{
  QTreeWidgetItemIterator it( listviewCommands );
  for ( ; *it; ++it )
    {
      QString command = (*it)->text( 1 );
      if( theExperiment.robot.getLangParam()->hasCommand( command ) )
	{
	  (*it)->setIcon( 0, QIcon( allow ) );
	  (*it)->setText( 2, QString::number( theExperiment.robot.getLangParam()->getCommand( command )->getDuration() ) );
	}
      else
	{
	  (*it)->setIcon( 0, QIcon( disallow ) );
	  (*it)->setText( 2, "0" );
	}
    }

  spinboxNumberOfRegisters->setValue( theExperiment.robot.getLangParam()->getMemorySize() );
  spinboxRegisterWidth->setValue( theExperiment.robot.getLangParam()->getRegisterWidth() );
  spinboxMaximalDelayTime->setValue( theExperiment.robot.getLangParam()->getMaximalDelayTime() );
};

void SIG_LanguageParameters::slotPushButtonEditClicked()
{
  slotCommandDoubleClicked( listviewCommands->currentItem() );
};

void SIG_LanguageParameters::slotPushButtonAllowAllClicked()
{
  QTreeWidgetItemIterator it( listviewCommands );
  for ( ; *it; ++it )
    {
      QString command = (*it)->text( 1 );
      if( !theExperiment.robot.getLangParam()->hasCommand( command ) )
	theExperiment.robot.getLangParam()->addCommand( command, new SIGEL_Robot::SIG_CommandParameters() );
      SIGEL_Robot::SIG_CommandParameters *commandParameters = theExperiment.robot.getLangParam()->getCommand( command );
      commandParameters->setDuration( (*it)->text( 2 ).toDouble() );
      (*it)->setIcon( 0, QIcon( allow ) );
    }
};

void SIG_LanguageParameters::slotPushButtonDisallowAllClicked()
{
  QTreeWidgetItemIterator it( listviewCommands );
  for ( ; *it; ++it )
    {
      QString command = (*it)->text( 1 );
      theExperiment.robot.getLangParam()->removeCommand( command );
      (*it)->setIcon( 0, QIcon( disallow ) );
    }
};

void SIG_LanguageParameters::slotCommandDoubleClicked( QTreeWidgetItem *theItem )
{
  if( theItem )
    {
      // lets collect all selected commands
      int numberOfSelectedCommands = 0;
      QTreeWidgetItemIterator listIt( listviewCommands );
      for( ; *listIt; ++listIt )
	{
	  if( (*listIt)->isSelected() )
	    ++numberOfSelectedCommands;
	}
      
      QString command = theItem->text( 1 ); // HAS TO BE IN THE true-BRANCH OF IF!!!
      SIG_EditCommandDialog editCommandDialog( this, "EditCommandDialog", true );
      if( numberOfSelectedCommands == 1 )
	{
	  editCommandDialog.setWindowTitle( "Edit command " + command );
	  editCommandDialog.textlabelCommand->setText( "Command: " + command );
	  if( theExperiment.robot.getLangParam()->hasCommand( command ) )
	    editCommandDialog.radiobuttonAllow->setChecked( true );
	  else
	    editCommandDialog.radiobuttonDisallow->setChecked( true );
	  editCommandDialog.lineeditDuration->setText( theItem->text( 2 ) );
	  editCommandDialog.lineeditDuration->setFocus();
	}
      else
	{
	  editCommandDialog.setWindowTitle( "Edit commands..." );
	  editCommandDialog.textlabelCommand->hide( );
	  if( theExperiment.robot.getLangParam()->hasCommand( command ) )
	    editCommandDialog.radiobuttonAllow->setChecked( true );
	  else
	    editCommandDialog.radiobuttonDisallow->setChecked( true );
	  editCommandDialog.lineeditDuration->setText( theItem->text( 2 ) );
	  editCommandDialog.lineeditDuration->setFocus();
	  editCommandDialog.resize( QSize() );
	}

      QTreeWidgetItemIterator listIt2( listviewCommands );
      
      switch( editCommandDialog.exec() )
	{
	  // the OK button was clicked
	case QDialog::Accepted:
	  // iterate over the listview
	  for( ; *listIt2; listIt2++ )
	    {
	      // check if the current is selected
	      if( (*listIt2)->isSelected() )
		{
		  // which command are we in currently
		  QString currentCommand = (*listIt2)->text( 1 );
		  // check if the command is allowed
		  if( editCommandDialog.radiobuttonDisallow->isChecked() )
		    {
		      // the command is not allowed...
		      theExperiment.robot.getLangParam()->removeCommand( currentCommand );
		      (*listIt2)->setIcon( 0, QIcon( disallow ) );
		      (*listIt2)->setText( 2, editCommandDialog.lineeditDuration->text() );
		    }
		  else
		    {
		      // we will work on command parameters, so we need this pointer.
		      SIGEL_Robot::SIG_CommandParameters *commandParameters = 0;
		      
		      // lets see if we first need to add the command and create a new SIG_CommandParameters object...
		      if( !theExperiment.robot.getLangParam()->hasCommand( currentCommand ) )
			// there is no such command, so we first have to add it...
			theExperiment.robot.getLangParam()->addCommand( currentCommand, new SIGEL_Robot::SIG_CommandParameters() );
		      
		      // lets enter the data into the SIG_CommandParameters object associated with the command
		      commandParameters = theExperiment.robot.getLangParam()->getCommand( currentCommand );
		      commandParameters->setDuration( editCommandDialog.lineeditDuration->text().toDouble() );
		      
		      // lets enter the data into the GUI
		      (*listIt2)->setIcon( 0, QIcon( allow ) );
		      (*listIt2)->setText( 2, editCommandDialog.lineeditDuration->text() );
		    } // end of else
		} // end of second if
	    } // end for
	  break;
	  
	case QDialog::Rejected:
	  break;
	}
    }
};

}
