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
#ifndef SIGEL_MASTERGUI_SIG_EXPERIMENTITEM
#define SIGEL_MASTERGUI_SIG_EXPERIMENTITEM

#include <qlistview.h>
#include <qstring.h>

namespace SIGEL_MasterGUI
{
  /**
   * The class representing an experiment in the ListView.
   *
   * A SIG_ExperimentItem knows how create all options, settings, which 
   * pixmaps to load etc.
   */
  class SIG_ExperimentItem : public QListViewItem
    {
    public:
      /**
       * The constructor of an SIG_ExperimentItem.
       *
       * An SIG_ExperimentItem knows how to build itself.
       * @param parent The ListView into which the item is put in.
       * @param name The name of the experiment. The first ListViewItem
       * will get this as its text.
       */
      SIG_ExperimentItem( QListView * parent, QString name );

      /**
       * The destructor of the SIG_ExperimentItem.
       *
       * Will be erased if not needed.
       */
      ~SIG_ExperimentItem();
    };
  
}
#endif // SIGEL_MASTERGUI_SIG_EXPERIMENTITEM_H
