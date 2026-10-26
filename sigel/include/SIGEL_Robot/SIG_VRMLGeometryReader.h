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
#ifndef SIGEL_ROBOT_SIG_VRMLGEOMETRYREADER_H
#define SIGEL_ROBOT_SIG_VRMLGEOMETRYREADER_H

#include <QList>
#include <QString>
#include <newmat.h>

namespace SIGEL_Robot
{

	/**
	 * Reads the geometry of a body from a VRML file.
	 */
	class SIG_VRMLGeometryReader
	{
	public:

		/**
		 * The line of the first USE that only whitespace or comments separate
		 * from a "}" before it, or 0; a USE in a string or comment does not count.
		 *
		 * CyberVRML97 runs a USE while its parser still holds the node that a
		 * "}" just before it closes, so the USE lands inside that node. Where
		 * the node is the one the USE names, it contains itself and
		 * Node::createInstanceNode aborts on an assertion; elsewhere the copy
		 * sits in the wrong place and the geometry is wrong. A comma after the
		 * "}" lets the parser close the node first. Tokens as in vrml.l.
		 */
		static int lineOfUseAfterNodeEnd( QString const &vrml );

		/** rotation holds the axis x, y, z and then the angle, as VRML gives them. */
		static NEWMAT::Matrix inverseVRMLRotationMatrix( QList< float > const &rotation );

	private:

		static qsizetype indexAfterComment( QString const &vrml, qsizetype start );

		static qsizetype indexAfterString( QString const &vrml, qsizetype start );

		static qsizetype indexAfterWord( QString const &vrml, qsizetype start );

		static bool isPartOfWord( QChar c );
	};

}

#endif // SIGEL_ROBOT_SIG_VRMLGEOMETRYREADER_H
