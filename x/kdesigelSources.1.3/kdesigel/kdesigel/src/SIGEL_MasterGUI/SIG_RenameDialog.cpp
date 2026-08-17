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
** Form implementation generated from reading ui file 'SIG_RenameDialog.ui'
**
** Created: Thu May 17 16:24:14 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SIGEL_MasterGUI/SIG_RenameDialog.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_RenameDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SIG_RenameDialog::SIG_RenameDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SIG_RenameDialog" );
    resize( 226, 92 ); 
    setCaption( tr( "Rename" ) );
    SIG_RenameDialogLayout = new QVBoxLayout( this ); 
    SIG_RenameDialogLayout->setSpacing( 6 );
    SIG_RenameDialogLayout->setMargin( 11 );

    Layout2 = new QHBoxLayout; 
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    textlabelNewName = new QLabel( this, "textlabelNewName" );
    textlabelNewName->setText( tr( "New name:" ) );
    Layout2->addWidget( textlabelNewName );

    lineeditNewName = new QLineEdit( this, "lineeditNewName" );
    Layout2->addWidget( lineeditNewName );
    SIG_RenameDialogLayout->addLayout( Layout2 );

    Layout3 = new QHBoxLayout; 
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout3->addItem( spacer );

    pushbuttonOK = new QPushButton( this, "pushbuttonOK" );
    pushbuttonOK->setText( tr( "&OK" ) );
    pushbuttonOK->setDefault( true );
    Layout3->addWidget( pushbuttonOK );

    pushbuttonCancel = new QPushButton( this, "pushbuttonCancel" );
    pushbuttonCancel->setText( tr( "&Cancel" ) );
    Layout3->addWidget( pushbuttonCancel );
    SIG_RenameDialogLayout->addLayout( Layout3 );

    // signals and slots connections
    connect( pushbuttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( pushbuttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_RenameDialog::~SIG_RenameDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

}
