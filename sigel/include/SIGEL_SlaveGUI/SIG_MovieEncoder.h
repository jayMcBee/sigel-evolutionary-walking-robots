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
#ifndef SIGEL_SLAVEGUI_SIG_MOVIEENCODER_H
#define SIGEL_SLAVEGUI_SIG_MOVIEENCODER_H

#include "SIGEL_SlaveGUI/SIG_MovieSettings.h"

#include <QProcess>
#include <QString>
#include <QStringList>

class QWidget;

namespace SIGEL_SlaveGUI
{

	/**
	 * Tells the user what a stopped recording wrote, and makes an MP4 of the
	 * frames with ffmpeg if the user wants one.
	 */
	class SIG_MovieEncoder
	{
	public:

		SIG_MovieEncoder( QWidget *messageParent );

		void encode( SIG_MovieSettings const &settings, int frameCount );

	private:

		// Runs ffmpeg behind a modal busy dialog and returns when ffmpeg has ended.
		void runFfmpegWithProgressDialog( QString const &ffmpeg, SIG_MovieSettings const &settings, int frameCount );

		QStringList arguments( SIG_MovieSettings const &settings, int frameCount ) const;

		void reportFinished();

		QWidget *messageParent;

		QProcess process;

		QString movieFileName;

		// ffmpeg writes here; the file is renamed to movieFileName when ffmpeg has succeeded.
		QString unfinishedFileName;
	};

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIEENCODER_H
