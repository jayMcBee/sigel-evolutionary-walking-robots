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
#ifndef SIGEL_ROBOT_SIG_VRMLFILEERROR_H
#define SIGEL_ROBOT_SIG_VRMLFILEERROR_H

#include <QString>
#include "SIGEL_Tools/SIG_Exception.h"

namespace SIGEL_Robot
{

	/**
	 * A VRML file that cannot be read or parsed.
	 */
	class SIG_VRMLFileError : public SIGEL_Tools::SIG_Exception
	{
	public:

		SIG_VRMLFileError( QString file, int line, QString vrmlFile, int vrmlLine, QString message )
			: SIG_Exception( file, line, "VRML file \"" + vrmlFile + "\", line " + QString::number( vrmlLine ) + ": " + message )
		{ }

		SIG_VRMLFileError( QString file, int line, QString vrmlFile, QString message )
			: SIG_Exception( file, line, "VRML file \"" + vrmlFile + "\": " + message )
		{ }
	};

}

#endif // SIGEL_ROBOT_SIG_VRMLFILEERROR_H
