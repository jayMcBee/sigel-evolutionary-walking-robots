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
#ifndef SIGEL_MASTERGUI_SIG_INDIVIDUALLISTITEM_H
#define SIGEL_MASTERGUI_SIG_INDIVIDUALLISTITEM_H

#include <QTreeWidget>

#include "SIGEL_GP/SIG_GPIndividual.h"

namespace SIGEL_MasterGUI
{
  /**
   * The class representing an individual in the GUI.
   *
   * This class inherits from QTreeWidgetItem. Information
   * about this class can be found in the Qt reference
   * library.
   */
class SIG_IndividualListItem : public QTreeWidgetItem
{
 public:
  
  /**
   * The constructor for creating an empty SIG_IndividualListItem.
   *
   * @param parent The QTreeWidget in which this item shall be displayed.
   */
  SIG_IndividualListItem( QTreeWidget *parent );

  /**
   * The constructor for creating a SIG_IndividualListItem displaying
   * information about theIndividual.
   *
   * @param parent The QTreeWidget in which this item shall be displayed.
   * @param theIndividual The gp individual whose information shall be
   * displayed by this item.
   */
  SIG_IndividualListItem( QTreeWidget *parent, int poolPosition, SIGEL_GP::SIG_GPIndividual *theIndividual );

  /**
   * The destructor.
   *
   * Will be erased if not needed.
   */
  ~SIG_IndividualListItem();

  /**
   * Inherited function used to sort the entries properly
   */
  QString key(int, bool) const;

  /**
   * The pool-position of the SIG_GPIndividual this list item represents.
   */
  int poolPosition;

  /**
   * This method lets the item display information about theIndividual.
   *
   * @param theIndividual The SIG_GPIndividual whose information shall be
   * displayed by the item.
   */
  void setTo( SIGEL_GP::SIG_GPIndividual *theIndividual );

  /**
   * A pointer to the SIG_GPIndividual this list item represents.
   */
  SIGEL_GP::SIG_GPIndividual *theIndividual;
};

}

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALLISTITEM_H
