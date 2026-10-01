/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_SLAVEGUI_SIG_MOVIESTATICRUNINFO_H
#define SIGEL_SLAVEGUI_SIG_MOVIESTATICRUNINFO_H

#include <QFileInfo>
#include <QString>

namespace SIGEL_SlaveGUI
{

  /**
   * What a simulation shows, for the overlay labels of a movie. It stays
   * the same for the whole simulation.
   */
  struct SIG_MovieStaticRunInfo
  {
    /**
     * Empty when the name is not known; its overlay label is then not drawn.
     */
    QString experimentName;

    /**
     * Takes the experiment name from the experiment's file name: without
     * the directory and without ".exp".
     */
    void setExperimentFileName( QString const &fileName )
    {
      experimentName = QFileInfo( fileName ).fileName();

      if ( experimentName.endsWith( ".exp" ) )
        experimentName.chop( 4 );
    }
  };

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIESTATICRUNINFO_H
