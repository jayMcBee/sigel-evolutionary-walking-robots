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
#include "SIGEL_MasterGUI/SIG_ExperimentItem.h"
#include <qpixmap.h>

#include <cstdlib>

SIGEL_MasterGUI::SIG_ExperimentItem::SIG_ExperimentItem( QListView * parent, QString name ) : QListViewItem(parent)
{
#ifdef _WINDOWS
  char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif
  
  QString sigelRootString( sigelRootCString );

  setText(0, name);
  setPixmap(0, QPixmap( sigelRootString + "/pixmaps/experimentSmall.xpm" ) );
  QListViewItem *newItem = new QListViewItem( this );

  // environment
  newItem->setText(0, "Environment");
  newItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/environSmall.xpm" ) );
  
  // newItem = new QListViewItem( this );
  // newItem->setText( 0, "Parameters");

  // language parameters
  
  newItem = new QListViewItem( this );
  newItem->setText(0, "Simulation-Parameters");
  newItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/simulationParameterSmall.xpm" ));
 
  newItem = new QListViewItem( this );
  newItem->setText(0, "GP-Parameters");
  newItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/dnaSmall.xpm") );

  // robot
  newItem = new QListViewItem( this );
  newItem->setText(0, "Robot");
  newItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/robotSmall.xpm" ) );
  
  QListViewItem *childItem = new QListViewItem( newItem );
  childItem->setText(0, "Language-Parameters");
  childItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/balloonSmall.xpm" ) );
  
  newItem->setOpen( true );

  // Individuals
  newItem = new QListViewItem( this );
  newItem->setText(0, "Individuals");
  newItem->setPixmap(0, QPixmap( sigelRootString + "/pixmaps/individualSmall.xpm" ) );

  this->setOpen( true );
};

SIGEL_MasterGUI::SIG_ExperimentItem::~SIG_ExperimentItem(){};
