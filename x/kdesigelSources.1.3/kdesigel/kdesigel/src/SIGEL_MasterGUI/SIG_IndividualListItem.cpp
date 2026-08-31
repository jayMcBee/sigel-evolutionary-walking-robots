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
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"

#include <qpixmap.h>

#include <cstdlib>

namespace SIGEL_MasterGUI
{

SIG_IndividualListItem::SIG_IndividualListItem( QTreeWidget *parent )
  : QTreeWidgetItem()
{
  // Qt 2's QListViewItem( QListView * ) PREPENDED (qlistview.cpp:585);
  // Qt 6's QTreeWidgetItem( QTreeWidget * ) appends, so the item is built
  // detached and inserted at the head instead. Nothing listens to
  // itemChanged, so doing it first -- as Qt 2 did, before the body ran --
  // is unobservable either way.
  parent->insertTopLevelItem( 0, this );
#ifdef _WINDOWS
  QString sigelRoot( ::getenv( "SIGEL_ROOT" ) );
#else
  QString sigelRoot( std::getenv( "SIGEL_ROOT" ) );
#endif
  setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/individualSmall.xpm" ) ) );
};

SIG_IndividualListItem::SIG_IndividualListItem( QTreeWidget *parent, int poolPosition, SIGEL_GP::SIG_GPIndividual *theIndividual )
  : QTreeWidgetItem(), poolPosition( poolPosition), theIndividual( theIndividual ) 
{
  parent->insertTopLevelItem( 0, this );   // Qt 2 prepended; see above
#ifdef _WINDOWS
  QString sigelRoot( ::getenv( "SIGEL_ROOT" ) );
#else
  QString sigelRoot( std::getenv( "SIGEL_ROOT" ) );
#endif
  if( theIndividual )
    {
      QString name = theIndividual->getName();
      setText(0, name ); // has to be theIndividual->getName()
      QString fitness = QString::number( theIndividual->getFitness() );
      setText(1, fitness );
      QString age = QString::number( theIndividual->getAge() );
      setText(2, age );
      setIcon( 0, QIcon( QPixmap( sigelRoot + "/pixmaps/individualSmall.xpm" ) ) );
      this->theIndividual = theIndividual;
      this->poolPosition = theIndividual->getPoolPos();
    }
};

SIG_IndividualListItem::~SIG_IndividualListItem()
{
  
};

QString SIG_IndividualListItem::key(int column, bool ascending) const {
  int const zeros = 999;
  switch(column) {
    case 0: {
      QString result;
      int length = this->text(0).length();
      int difference = zeros - length;
      for( int i=0; i < difference; i++ )
        result.prepend("0");
      result.append(this->text(0));
      return result;
      }
      break;
    case 1: {
      QString result, orgString;
      int positionOfPoint = this->text(1).indexOf( "." );
      int positionOfExponent = this->text(1).indexOf("e");
      orgString = this->text(1);
      if ( positionOfPoint != -1 ) {
        orgString.truncate(positionOfExponent);
        orgString.remove(positionOfPoint,1);
        positionOfExponent = (positionOfExponent != -1) ? (this->text(1).right(this->text(1).length()-(positionOfExponent+1))).toInt() : 0;
        for( int i=0; i < zeros - positionOfPoint - positionOfExponent; i++ )
          result.prepend("0");
      }
      else {
        positionOfExponent = (positionOfExponent != -1) ? (this->text(1).right(this->text(1).length()-(positionOfExponent+1))).toInt() : 0;
        for( int i=0; i < zeros - this->text(1).length() - positionOfExponent; i++ )
          result.prepend("0");
      }
      result.append(orgString);
      return result;
      }
      break;
    case 2: {
      QString result;
      int length = this->text(2).length();
      int difference = zeros - length;
      for( int i=0; i < difference; i++ )
        result.prepend("0");
      result.append(this->text(2));
      return result;
      }
      break;
  }
  return QString();
};

void SIG_IndividualListItem::setTo( SIGEL_GP::SIG_GPIndividual *theIndividual )
{
  if( theIndividual )
    {
      QString name = theIndividual->getName();
      setText(0, name ); // has to be theIndividual->getName()
      QString fitness = QString::number( theIndividual->getFitness() );
      setText(1, fitness );
      QString age = QString::number( theIndividual->getAge() );
      setText(2, age );
      //  setPixmap( 0, QPixmap( "./pixmaps/individualSmall.xpm" ) );
      this->theIndividual = theIndividual;
      this->poolPosition = theIndividual->getPoolPos();
    }
};

}
