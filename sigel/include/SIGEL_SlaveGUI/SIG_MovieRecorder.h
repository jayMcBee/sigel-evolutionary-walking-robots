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
   * Writes the frames of a movie of the simulation, numbered, at the frame
   * rate of its settings in simulated time.
   */
  class SIG_MovieRecorder
  {
  public:

    SIG_MovieRecorder();

    SIG_MovieSettings const &getSettings() const { return settings; }

    void setSettings( SIG_MovieSettings const &newSettings );

    bool isRecording() const { return recording; }

    // Starts recording, or restarts the frame timing, at simulationSeconds.
    void startRecordingAt( double simulationSeconds );

    void stopRecording();

    // True if a frame is due at simulationSeconds.
    bool needsToRecordFrameAt( double simulationSeconds ) const;

    // Saves the view centred in a frame of the movie size, never scaled.
    bool writeImage( QImage const &view, double simulationSeconds );

    // Writes the frame as a POV-Ray scene file.
    bool writePovray( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation, double simulationSeconds );

    // Writes the include file that every POV-Ray scene file reads.
    bool createPovrayIncludeFile( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation );

    // Counts a frame that could not be written, too.
    int getFramesRecorded() const { return framesRecorded; }

    QString getLastFileName() const { return lastFileName; }

    // Stops recording and numbers frames from 0 again.
    void reset();

  private:

    QString nextFrameFileName() const;

    // Names and counts the frame, times the next one, and makes the directory.
    void startFrame( double simulationSeconds );

    void makeDirectory() const;

    // The frame timing's position at simulationSeconds, in frames.
    double framePosition( double simulationSeconds ) const;

    SIG_MovieSettings settings;

    bool recording;

    int framesRecorded;

    double recordingStartSeconds;

    // The due frame, counted in frames from recordingStartSeconds.
    int nextFrame;

    QString lastFileName;

  };

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIERECORDER_H
