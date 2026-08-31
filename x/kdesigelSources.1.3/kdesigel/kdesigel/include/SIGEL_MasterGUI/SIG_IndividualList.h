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
#ifndef SIGEL_MASTERGUI_SIG_INDIVIDUALLIST
#define SIGEL_MASTERGUI_SIG_INDIVIDUALLIST

#include <QMenu>

#include "SIGEL_MasterGUI/SIG_IndividualListBase.h"

namespace SIGEL_MasterGUI
{
  /**
   * The class of the widget, in which the individuals will be shown.
   *
   * This class inherits from SIG_IndividualListBase which was created
   * with the Qt designer.
   */
class SIG_IndividualList : public SIG_IndividualListBase
{
  Q_OBJECT

 public:

  /**
   * The constructor of the individual list.
   *
   * @param parent The parent widget og SIG_IndividualList. If 0 the
   * widget will get its own window.
   * @param name Internal name for Qt.
   */
  SIG_IndividualList( QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags() );
};

}

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALLIST
