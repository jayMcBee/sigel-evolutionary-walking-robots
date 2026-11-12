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
#include <QDialogButtonBox>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_AddIndividualsDialogBase which is a child of 'parent', with
 *  widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SIG_AddIndividualsDialog::SIG_AddIndividualsDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setModal( modal );

    setObjectName( "SIG_AddIndividualsDialogBase" );
    resize( 300, 95 ); 
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum ) );
    setWindowTitle( tr( "Add Individuals" ) );
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
    spinboxNumber->setMaximum( 9999 );
    spinboxNumber->setMinimum( 1 );
    spinboxNumber->setValue( 1 );
    Layout4->addWidget( spinboxNumber );
    SIG_AddIndividualsDialogBaseLayout->addLayout( Layout4 );

    QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this );
    connect( buttons, &QDialogButtonBox::accepted, this, &QDialog::accept );
    connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );
    SIG_AddIndividualsDialogBaseLayout->addWidget( buttons );
}

}
