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
#include "SIGEL_SlaveGUI/SIG_MovieEncoder.h"

#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>

namespace SIGEL_SlaveGUI
{

	SIG_MovieEncoder::SIG_MovieEncoder( QWidget *messageParent )
		: messageParent( messageParent )
	{
		QObject::connect( &process, &QProcess::finished, [this]() { reportFinished(); } );

		// A process that fails to start never sends finished.
		QObject::connect( &process, &QProcess::errorOccurred, [this]( QProcess::ProcessError error )
		{
			if ( error == QProcess::FailedToStart )
				reportFinished();
		} );
	};

	SIG_MovieEncoder::~SIG_MovieEncoder()
	{
		// ~QProcess kills a running ffmpeg and sends finished, when this is half destroyed.
		QObject::disconnect( &process, nullptr, nullptr, nullptr );
	};

	void SIG_MovieEncoder::encode( SIG_MovieSettings const &settings, int frameCount )
	{
		if ( frameCount == 0 )
		{
			QMessageBox::information( messageParent, "Recording Stopped", "Recording stopped. No frames were written." );
			return;
		}

		QString written = QString( "%1 frames written to:\n%2" ).arg( frameCount ).arg( settings.directory );

		if ( settings.format == "pov" )
		{
			QMessageBox::information( messageParent, "Recording Stopped", written );
			return;
		}

		QString ffmpeg = QStandardPaths::findExecutable( "ffmpeg" );

		if ( ffmpeg.isEmpty() )
		{
			QMessageBox::information( messageParent, "Recording Stopped", written + "\n\nffmpeg was not found, so no MP4 was made." );
			return;
		}

		if ( process.state() != QProcess::NotRunning )
		{
			QMessageBox::information( messageParent, "Recording Stopped", written + "\n\nffmpeg is still making the last movie, so no MP4 was made." );
			return;
		}

		QString movieName = settings.filePrefix + ".mp4";
		QString question = written + "\n\nMake " + movieName + " from them?";

		if ( QFile::exists( settings.directory + movieName ) )
			question += "\nThis replaces the existing file.";

		if ( QMessageBox::question( messageParent, "Recording Stopped", question ) != QMessageBox::Yes )
			return;

		movieFileName = settings.directory + movieName;
		process.start( ffmpeg, arguments( settings, frameCount ) );
	};

	QStringList SIG_MovieEncoder::arguments( SIG_MovieSettings const &settings, int frameCount ) const
	{
		// ffmpeg reads % in the input name as a pattern.
		QString input = QString( settings.directory + settings.filePrefix ).replace( "%", "%%" );

		// The same number width as SIG_MovieRecorder::nextFrameFileName.
		if ( settings.useLeadingZeros )
			input += "%0" + QString::number( QString::number( settings.maxFrames ).length() ) + "d";
		else
			input += "%d";

		input += "." + settings.format;

		// libx264 with yuv420p needs an even width and height. Players take an
		// HD movie as BT.709, so the frames are converted and tagged as that.
		// faststart puts the index first, so the movie plays while it loads.
		return { "-y", "-framerate", QString::number( settings.frameRate ), "-start_number", "0",
			"-i", input, "-frames:v", QString::number( frameCount ),
			"-vf", "crop=trunc(iw/2)*2:trunc(ih/2)*2,scale=out_color_matrix=bt709,setparams=colorspace=bt709:color_primaries=bt709:color_trc=bt709",
			"-c:v", "libx264", "-crf", "18", "-pix_fmt", "yuv420p",
			"-movflags", "+faststart", settings.directory + settings.filePrefix + ".mp4" };
	};

	void SIG_MovieEncoder::reportFinished()
	{
		if ( process.error() != QProcess::FailedToStart && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 )
		{
			QMessageBox::information( messageParent, "Movie Written", "Movie written to:\n" + movieFileName );
			return;
		}

		QString errorOutput = QString::fromLocal8Bit( process.readAllStandardError() ).trimmed();

		if ( errorOutput.isEmpty() )
			errorOutput = process.errorString();

		QStringList errorLines = errorOutput.split( '\n' );

		QMessageBox::warning( messageParent, "Movie Error",
			"ffmpeg could not make:\n" + movieFileName + "\n\n" + errorLines.mid( qMax( 0, errorLines.size() - 5 ) ).join( '\n' ) );
	};

}
