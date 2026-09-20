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
** Form implementation generated from reading ui file 'SIG_AddIndividualsDialogBase.ui'
**
** Created: Thu Mar 22 12:26:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "SIGEL_MasterGUI/SIG_AddIndividualsDialog.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_AddIndividualsDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SIG_AddIndividualsDialog::SIG_AddIndividualsDialog( QWidget* parent,  const char* name, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setObjectName( QString::fromUtf8( name ) );
    setModal( modal );

    if ( !name )
	setObjectName( "SIG_AddIndividualsDialogBase" );
    resize( 222, 92 ); 
    setSizePolicy( QSizePolicy( (QSizePolicy::Policy)5, (QSizePolicy::Policy)1 ) );
    setWindowTitle( tr( "Add individuals..." ) );
    SIG_AddIndividualsDialogBaseLayout = new QVBoxLayout( this ); 
    SIG_AddIndividualsDialogBaseLayout->setSpacing( 6 );
    SIG_AddIndividualsDialogBaseLayout->setContentsMargins( 11, 11, 11, 11 );

    Layout4 = new QHBoxLayout; 
    Layout4->setSpacing( 6 );
    Layout4->setContentsMargins( 0, 0, 0, 0 );

    textlabelNumber = new QLabel( this );
    textlabelNumber->setText( tr( "Number to add:" ) );
    Layout4->addWidget( textlabelNumber );

    spinboxNumber = new QSpinBox( this );
    spinboxNumber->setMaximum( 999 );
    spinboxNumber->setMinimum( 1 );
    spinboxNumber->setValue( 1 );
    Layout4->addWidget( spinboxNumber );
    SIG_AddIndividualsDialogBaseLayout->addLayout( Layout4 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setContentsMargins( 0, 0, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    pushbuttonOK = new QPushButton( this );
    pushbuttonOK->setText( tr( "&OK" ) );
    pushbuttonOK->setDefault( true );
    Layout1->addWidget( pushbuttonOK );

    pushbuttonCancel = new QPushButton( this );
    pushbuttonCancel->setText( tr( "&Cancel" ) );
    Layout1->addWidget( pushbuttonCancel );
    SIG_AddIndividualsDialogBaseLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( pushbuttonOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( pushbuttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
};

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_AddIndividualsDialog::~SIG_AddIndividualsDialog()
{
    // no need to delete child widgets, Qt does it all for us
};

}
