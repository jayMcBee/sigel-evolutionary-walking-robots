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
#ifndef SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H
#define SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H

#include <qstring.h>

namespace SIGEL_Visualisation
{

  class SIG_SimulationVisualisation;

  /**
   * Writes a SIG_SimulationVisualisation as POV-Ray scene files: one
   * include file with the declarations of the plane, the grid and the
   * robot, and one scene file per frame.
   */
  class SIG_PovrayExporter
  {
  public:
    SIG_PovrayExporter( SIG_SimulationVisualisation const &visualisation );

    /**
     * Writes the include file that every frame includes.
     *
     * @return false if the file cannot be written.
     */
    bool writeIncludeFile( QString fileName, double aspectRatio ) const;

    /**
     * Writes one frame: the scene as it is now.
     *
     * @return false if the file cannot be written.
     */
    bool writeFrame( QString includeFileName, QString fileName ) const;

  private:
    QString environmentDeclarations() const;

    SIG_SimulationVisualisation const &visualisation;
  };

}

#endif // SIGEL_VISUALISATION_SIG_POVRAYEXPORTER_H
