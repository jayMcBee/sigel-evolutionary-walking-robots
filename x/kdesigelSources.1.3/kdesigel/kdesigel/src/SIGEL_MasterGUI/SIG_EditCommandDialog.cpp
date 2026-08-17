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
/****************************************************************************
** Form implementation generated from reading ui file 'SIG_EditCommandDialog.ui'
**
** Created: Mon May 7 12:02:05 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SIGEL_MasterGUI/SIG_EditCommandDialog.h"

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvalidator.h>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_EditCommandDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SIG_EditCommandDialog::SIG_EditCommandDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SIG_EditCommandDialog" );
    resize( 222, 201 ); 
    setCaption( tr( "Edit Command" ) );
    SIG_EditCommandDialogLayout = new QVBoxLayout( this ); 
    SIG_EditCommandDialogLayout->setSpacing( 6 );
    SIG_EditCommandDialogLayout->setMargin( 11 );

    textlabelCommand = new QLabel( this, "textlabelCommand" );
    textlabelCommand->setText( tr( "Command:" ) );
    SIG_EditCommandDialogLayout->addWidget( textlabelCommand );

    buttongroupAllowDisallow = new QButtonGroup( this, "buttongroupAllowDisallow" );
    buttongroupAllowDisallow->setTitle( tr( "Allow/Disallow" ) );
    buttongroupAllowDisallow->setColumnLayout(0, Qt::Vertical );
    buttongroupAllowDisallow->layout()->setSpacing( 0 );
    buttongroupAllowDisallow->layout()->setMargin( 0 );
    buttongroupAllowDisallowLayout = new QVBoxLayout( buttongroupAllowDisallow->layout() );
    buttongroupAllowDisallowLayout->setAlignment( Qt::AlignTop );
    buttongroupAllowDisallowLayout->setSpacing( 6 );
    buttongroupAllowDisallowLayout->setMargin( 11 );

    radiobuttonAllow = new QRadioButton( buttongroupAllowDisallow, "radiobuttonAllow" );
    radiobuttonAllow->setText( tr( "Allow" ) );
    radiobuttonAllow->setChecked( TRUE );
    buttongroupAllowDisallowLayout->addWidget( radiobuttonAllow );

    radiobuttonDisallow = new QRadioButton( buttongroupAllowDisallow, "radiobuttonDisallow" );
    radiobuttonDisallow->setText( tr( "Disallow" ) );
    buttongroupAllowDisallowLayout->addWidget( radiobuttonDisallow );
    SIG_EditCommandDialogLayout->addWidget( buttongroupAllowDisallow );

    Layout2 = new QHBoxLayout; 
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    textlabelDuration = new QLabel( this, "textlabelDuration" );
    textlabelDuration->setText( tr( "Duration:" ) );
    Layout2->addWidget( textlabelDuration );

    lineeditDuration = new QLineEdit( this, "lineeditDuration" );
    lineeditDuration->setValidator( new QDoubleValidator( this, "lineeditDurations DoubleValidator" ) );
    Layout2->addWidget( lineeditDuration );
    SIG_EditCommandDialogLayout->addLayout( Layout2 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    pushbuttonOK = new QPushButton( this, "pushbuttonOK" );
    pushbuttonOK->setText( tr( "&OK" ) );
    pushbuttonOK->setDefault( true );
    Layout1->addWidget( pushbuttonOK );

    pushbuttonCancel = new QPushButton( this, "pushbuttonCancel" );
    pushbuttonCancel->setText( tr( "&Cancel" ) );
    Layout1->addWidget( pushbuttonCancel );
    SIG_EditCommandDialogLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( pushbuttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( pushbuttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
};

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_EditCommandDialog::~SIG_EditCommandDialog()
{
    // no need to delete child widgets, Qt does it all for us
};

}
