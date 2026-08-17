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
#include <qlabel.h>
#include <qmultilineedit.h>

#include "SIGEL_MasterGUI/SIG_IndividualView.h"

#include "SIGEL_Tools/SIG_IO.h"

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_IndividualView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_IndividualView::SIG_IndividualView( QWidget* parent,  const char* name, WFlags fl )
    : SIG_IndividualViewBase( parent, name, fl )
{
}

SIG_IndividualView::SIG_IndividualView( QWidget* parent,  const char* name, WFlags fl, SIGEL_GP::SIG_GPIndividual *theGPIndividual )
    : SIG_IndividualViewBase( parent, name, fl )
{
  textlabelShowName->setText( theGPIndividual->getName() );
  QString age = QString::number( theGPIndividual->getAge() );
  textlabelShowAge->setText( age );
  QString fitness = QString::number( theGPIndividual->getFitness() );
  textlabelShowFitness->setText( fitness );
  QString programCode;
  theGPIndividual->getProgramPointer()->printToString( programCode );
  multilineeditProgramCode->setText( programCode );
  QString history = theGPIndividual->getHistory().join( "\n" );
  multilineeditHistory->setText( history );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_IndividualView::~SIG_IndividualView()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_IndividualView::clear()
{
  textlabelShowName->setText( QString::null );
  textlabelShowAge->setText( QString::null );
  textlabelShowFitness->setText( QString::null );
  multilineeditProgramCode->clear();
  multilineeditHistory->clear();
};

/* void SIG_IndividualView::closeEvent( QCloseEvent *e )
{
  QWidget::closeEvent( e );
  delete this;
}; */

}
