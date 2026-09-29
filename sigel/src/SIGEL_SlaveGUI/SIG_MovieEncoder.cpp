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

#include <QMessageBox>

namespace SIGEL_SlaveGUI
{

	SIG_MovieEncoder::SIG_MovieEncoder( QWidget *messageParent )
		: messageParent( messageParent )
	{
	};

	void SIG_MovieEncoder::encode( SIG_MovieSettings const &settings, int frameCount )
	{
		if ( frameCount == 0 )
		{
			QMessageBox::information( messageParent, "Recording Stopped", "Recording stopped. No frames were written." );
			return;
		}

		QMessageBox::information( messageParent, "Recording Stopped",
			QString( "%1 frames written to %2" ).arg( frameCount ).arg( settings.directory ) );
	};

}
