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
#ifndef SIGEL_SLAVEGUI_SIG_MOVIERECORDER_H
#define SIGEL_SLAVEGUI_SIG_MOVIERECORDER_H

#include "SIGEL_SlaveGUI/SIG_MovieSettings.h"
#include <QImage>
#include <QString>

namespace SIGEL_Visualisation
{
  class SIG_SimulationVisualisation;
}

namespace SIGEL_SlaveGUI
{

  /**
   * Writes the frames of a movie of the simulation: images or POV-Ray
   * scene files, numbered, into the directory of its settings.
   */
  class SIG_MovieRecorder
  {
  public:

    SIG_MovieRecorder();

    SIG_MovieSettings const &getSettings() const;

    void setSettings( SIG_MovieSettings const &newSettings );

    bool isRecording() const;

    void setRecording( bool newRecording );

    /**
     * Saves the view as the next frame of exactly the movie width x height,
     * pixel for pixel: a larger view is cut to its centre, a smaller one
     * is centred on black. Returns whether saving was successful.
     */
    bool writeImage( QImage const &view );

    /**
     * Exports the scene as the next frame, a POV-Ray scene file.
     * Returns whether exporting was successful.
     */
    bool writePovray( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation );

    /**
     * Writes the POV-Ray include file that every scene file of the movie
     * reads. Returns whether writing was successful.
     */
    bool createPovrayIncludeFile( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation );

    /**
     * The number of frames since the last reset, including any that
     * could not be written.
     */
    int getFramesRecorded() const;

    /**
     * The file the last write tried to write.
     */
    QString getLastFileName() const;

    /**
     * Stops recording and starts the frame numbers at 0 again.
     */
    void reset();

  private:

    /**
     * The file name of the next frame, with leading zeros if the
     * settings ask for them.
     */
    QString nextFrameFileName() const;

    /**
     * Names and counts the next frame, and makes the directory.
     */
    void startFrame();

    void makeDirectory() const;

    SIG_MovieSettings settings;

    bool recording;

    int framesRecorded;

    QString lastFileName;

  };

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIERECORDER_H
