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
#include "SIGEL_SlaveGUI/SIG_MovieRecorder.h"

#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <QDir>
#include <QPainter>

#include <cmath>

namespace SIGEL_SlaveGUI
{

  SIG_MovieRecorder::SIG_MovieRecorder()
    : settings(),
      recording( false ),
      framesRecorded( 0 ),
      recordingStartSeconds( 0.0 ),
      nextFrame( 0 ),
      lastFileName()
  {
  };

  void SIG_MovieRecorder::setSettings( SIG_MovieSettings const &newSettings )
  {
    settings = newSettings;
  };

  void SIG_MovieRecorder::startRecordingAt( double simulationSeconds )
  {
    recording = true;
    recordingStartSeconds = simulationSeconds;
    nextFrame = 0;
  };

  void SIG_MovieRecorder::stopRecording()
  {
    recording = false;
  };

  bool SIG_MovieRecorder::needsToRecordFrameAt( double simulationSeconds ) const
  {
    if ( !recording || framesRecorded >= settings.maxFrames )
      return false;

    return framePosition( simulationSeconds ) >= nextFrame;
  };

  bool SIG_MovieRecorder::writeImage( QImage const &view, double simulationSeconds )
  {
    startFrame( simulationSeconds );

    if ( view.isNull() )
      return false;

    // The view is in device pixels; drawn at ratio 1, they stay 1:1.
    QImage image = view;
    image.setDevicePixelRatio( 1.0 );

    QImage frame( settings.width, settings.height, QImage::Format_RGB32 );
    frame.fill( Qt::black );

    QPainter painter( &frame );
    painter.drawImage( (settings.width - image.width()) / 2,
		       (settings.height - image.height()) / 2,
		       image );
    painter.end();

    return frame.save( lastFileName,
		       settings.format.toUpper().toUtf8().constData(),
		       settings.quality );
  };

  bool SIG_MovieRecorder::writePovray( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation, double simulationSeconds )
  {
    startFrame( simulationSeconds );

    return visualisation.exportToPovray( settings.filePrefix + ".inc",
					 lastFileName );
  };

  bool SIG_MovieRecorder::createPovrayIncludeFile( SIGEL_Visualisation::SIG_SimulationVisualisation &visualisation )
  {
    lastFileName = settings.directory + settings.filePrefix + ".inc";
    makeDirectory();

    double aspectRatio = double( settings.width ) / double( settings.height );

    return visualisation.createPovrayIncludeFile( lastFileName, aspectRatio );
  };

  void SIG_MovieRecorder::reset()
  {
    recording = false;
    framesRecorded = 0;
  };

  QString SIG_MovieRecorder::nextFrameFileName() const
  {
    QString number = QString::number( framesRecorded );

    if ( settings.useLeadingZeros )
      number = number.rightJustified( QString::number( settings.maxFrames ).length(), '0' );

    return settings.directory + settings.filePrefix + number + "." + settings.format;
  };

  void SIG_MovieRecorder::startFrame( double simulationSeconds )
  {
    lastFileName = nextFrameFileName();
    framesRecorded++;
    // Frames a Fast Forward jumped over are skipped, not caught up.
    nextFrame = static_cast< int >( std::floor( framePosition( simulationSeconds ) ) ) + 1;
#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "Writing " << lastFileName << " in format " << settings.width << " x " << settings.height << " in quality " << settings.quality << "." << Qt::endl;
#endif
    makeDirectory();
  };

  double SIG_MovieRecorder::framePosition( double simulationSeconds ) const
  {
    // A step exactly on a frame time can come out a hair below it; the small
    // addition keeps that frame on this step.
    return ( simulationSeconds - recordingStartSeconds ) * settings.frameRate + 1e-6;
  };

  void SIG_MovieRecorder::makeDirectory() const
  {
    QDir movieDir( settings.directory );
    if ( !movieDir.exists() )
      movieDir.mkdir( settings.directory );
  };

}
