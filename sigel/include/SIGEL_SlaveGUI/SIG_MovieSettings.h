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
#ifndef SIGEL_SLAVEGUI_SIG_MOVIESETTINGS_H
#define SIGEL_SLAVEGUI_SIG_MOVIESETTINGS_H

#include <QString>

namespace SIGEL_SlaveGUI
{

  /**
   * The settings of a movie recording, as the movie settings dialog
   * edits them.
   */
  struct SIG_MovieSettings
  {
    int width = 1024;

    int height = 576;

    int frameRate = 25;

    int maxFrames = 1000;

    int quality = 50;

    bool useLeadingZeros = true;

    bool showOverlaySimulationTime = false;

    bool showOverlayStartDistance = false;

    bool showOverlayRobotHeight = false;

    bool saveThumbnails = false;

    /**
     * Ends with a slash once set.
     */
    QString directory;

    QString filePrefix = "sigel_pic";

    /**
     * The file extension in lower case: bmp, png, ppm, xbm, xpm or pov.
     */
    QString format = "png";
  };

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIESETTINGS_H
