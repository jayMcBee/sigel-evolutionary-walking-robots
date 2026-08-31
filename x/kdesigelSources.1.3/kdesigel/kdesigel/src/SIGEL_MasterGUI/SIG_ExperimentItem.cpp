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

SIGEL_MasterGUI::SIG_ExperimentItem::SIG_ExperimentItem( QTreeWidget * parent, QString name ) : QTreeWidgetItem()
{
  // Qt 2's QListViewItem( QListView * ) PREPENDED (qlistview.cpp:585);
  // Qt 6's QTreeWidgetItem( QTreeWidget * ) appends, so the item is built
  // detached and inserted at the head instead. Nothing listens to
  // itemChanged, so doing it first -- as Qt 2 did, before the body ran --
  // is unobservable either way.
  parent->insertTopLevelItem( 0, this );
#ifdef _WINDOWS
  char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif
  
  QString sigelRootString( sigelRootCString );

  setText(0, name);
  setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/experimentSmall.xpm" ) ) );
  QTreeWidgetItem *newItem = new QTreeWidgetItem();
  insertChild( 0, newItem );

  // environment
  newItem->setText(0, "Environment");
  newItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/environSmall.xpm" ) ) );
  
  // newItem = new QTreeWidgetItem( this );
  // newItem->setText( 0, "Parameters");

  // language parameters
  
  newItem = new QTreeWidgetItem();
  
  insertChild( 0, newItem );
  newItem->setText(0, "Simulation-Parameters");
  newItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/simulationParameterSmall.xpm" ) ) );
 
  newItem = new QTreeWidgetItem();
 
  insertChild( 0, newItem );
  newItem->setText(0, "GP-Parameters");
  newItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/dnaSmall.xpm") ) );

  // robot
  newItem = new QTreeWidgetItem();
  insertChild( 0, newItem );
  newItem->setText(0, "Robot");
  newItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/robotSmall.xpm" ) ) );
  
  QTreeWidgetItem *childItem = new QTreeWidgetItem();
  
  newItem->insertChild( 0, childItem );
  childItem->setText(0, "Language-Parameters");
  childItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/balloonSmall.xpm" ) ) );
  
  newItem->setExpanded( true );

  // Individuals
  newItem = new QTreeWidgetItem();
  insertChild( 0, newItem );
  newItem->setText(0, "Individuals");
  newItem->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/individualSmall.xpm" ) ) );

  this->setExpanded( true );
};

SIGEL_MasterGUI::SIG_ExperimentItem::~SIG_ExperimentItem(){};
