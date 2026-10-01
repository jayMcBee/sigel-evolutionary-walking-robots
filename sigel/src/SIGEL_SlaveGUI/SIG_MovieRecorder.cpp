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
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QStringList>

#include <cmath>

namespace SIGEL_SlaveGUI
{

  SIG_MovieRecorder::SIG_MovieRecorder()
    : settings(),
      recording( false ),
      framesRecorded( 0 ),
      recordingStartSeconds( 0.0 ),
      nextFrame( 0 ),
      lastFileName(),
      staticRunInfo()
  {
  };

  void SIG_MovieRecorder::setSettings( SIG_MovieSettings const &newSettings )
  {
    settings = newSettings;
  };

  void SIG_MovieRecorder::setStaticRunInfo( SIG_MovieStaticRunInfo const &newStaticRunInfo )
  {
    staticRunInfo = newStaticRunInfo;
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

  bool SIG_MovieRecorder::writeImage( QImage const &view, double simulationSeconds, double startDistance, double robotCentreHeight )
  {
    startFrame( simulationSeconds );

    if ( view.isNull() )
      return false;

    QImage frame = frameWithView( view );

    if ( !saveThumbnailOncePerSecond( frame ) )
      return false;

    paintOverlayLabels( frame, simulationSeconds, startDistance, robotCentreHeight );

    return frame.save( lastFileName, settings.format.toUpper().toUtf8().constData(), settings.quality );
  };

  QImage SIG_MovieRecorder::frameWithView( QImage const &view ) const
  {
    // The view is in device pixels; drawn at ratio 1, they stay 1:1.
    QImage image = view;
    image.setDevicePixelRatio( 1.0 );

    QImage frame( settings.width, settings.height, QImage::Format_RGB32 );
    frame.fill( Qt::black );

    QPainter painter( &frame );
    painter.drawImage( (settings.width - image.width()) / 2, (settings.height - image.height()) / 2, image );

    return frame;
  };

  bool SIG_MovieRecorder::saveThumbnailOncePerSecond( QImage const &frame )
  {
    // startFrame has already counted this frame.
    bool firstFrameOfSecond = (framesRecorded - 1) % settings.frameRate == 0;

    if ( !settings.saveThumbnails || !firstFrameOfSecond )
      return true;

    QString thumbnailName = thumbnailFileName();

    if ( frame.save( thumbnailName, "PNG" ) )
      return true;

    lastFileName = thumbnailName;
    return false;
  };

  void SIG_MovieRecorder::paintOverlayLabels( QImage &frame, double simulationSeconds, double startDistance, double robotCentreHeight ) const
  {
    // The font size is a fraction of the frame height; the other sizes are in font sizes.
    double const fontHeight = 20.0 / 720.0;
    double const lineStep = 31.0 / 22.0;
    double const margin = 16.0 / 22.0;
    double const valueGap = 0.5;
    double const outlineWidth = 1.0 / 20.0;
    QColor const textColor( 0xE8, 0xE8, 0xE8 );
    QColor const outlineColor( 0x40, 0x40, 0x40 );

    QStringList labels;
    QStringList values;

    if ( settings.showOverlayExperimentName && !staticRunInfo.experimentName.isEmpty() )
      {
        labels << "Experiment:";
        values << staticRunInfo.experimentName;
      }

    if ( settings.showOverlaySimulationTime )
      {
        labels << "Simulated time:";
        values << QString::number( simulationSeconds, 'f', 2 ) + " s";
      }

    if ( settings.showOverlayStartDistance )
      {
        labels << "Distance from start:";
        values << QString::number( startDistance, 'f', 3 ) + " m";
      }

    if ( settings.showOverlayRobotHeight )
      {
        labels << "Centre height:";
        values << QString::number( robotCentreHeight, 'f', 3 ) + " m";
      }

    if ( labels.isEmpty() )
      return;

    double fontSize = settings.height * fontHeight;
    QFont font = overlayFont( fontSize );

    QFontMetricsF metrics( font );
    double labelWidth = 0.0;
    for ( QString const &label : labels )
      labelWidth = qMax( labelWidth, metrics.horizontalAdvance( label ) );

    double marginPixels = fontSize * margin;
    double valueX = marginPixels + labelWidth + fontSize * valueGap;

    QPainterPath path;
    for ( int line = 0; line < labels.size(); line++ )
      {
        double baseline = marginPixels + fontSize + line * fontSize * lineStep;
        path.addText( marginPixels, baseline, font, labels[line] );
        path.addText( valueX, baseline, font, values[line] );
      }

    QPainter painter( &frame );
    painter.setRenderHint( QPainter::Antialiasing );

    // The pen is centred on the glyph edges, and the fill covers its inner half.
    QPen outlinePen( outlineColor, 2.0 * fontSize * outlineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    painter.strokePath( path, outlinePen );
    painter.fillPath( path, textColor );
  };

  QFont SIG_MovieRecorder::overlayFont( double pixelSize ) const
  {
    QFont font( "Noto Sans" );
    font.setWeight( QFont::Bold );

    // Ubuntu Sans comes with every Ubuntu desktop; its SemiBold matches Noto Sans Bold best.
    if ( !QFontDatabase::hasFamily( "Noto Sans" ) )
      {
        font.setFamily( "Ubuntu Sans" );
        font.setWeight( QFont::DemiBold );
      }

    font.setPixelSize( qMax( 1, qRound( pixelSize ) ) );

    return font;
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

  QString SIG_MovieRecorder::thumbnailFileName() const
  {
    int seconds = (framesRecorded - 1) / settings.frameRate;

    // "_thumb_" keeps the name out of the frame pattern that ffmpeg reads.
    return settings.directory + settings.filePrefix + "_thumb_"
      + QString::number( seconds ).rightJustified( 3, '0' ) + "s.png";
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
