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
#ifndef SIGEL_ROBOT_SIG_VRMLUTILS_H
#define SIGEL_ROBOT_SIG_VRMLUTILS_H

#include <QList>
#include <QString>
#include <newmat.h>

class SceneGraph;
class TransformNode;

namespace SIGEL_Robot
{

	/**
	 * Generic VRML functionality.
	 */
	class SIG_VRMLUtils
	{
	public:

		/**
		 * The line of the first USE that only whitespace or comments separate
		 * from a "}" before it, or 0. CyberVRML97 puts such a USE inside the
		 * node the "}" closes, which crashes it or misplaces the copy.
		 */
		static int lineOfUseAfterNodeEnd( QString const &vrml );

		/** rotation holds the axis x, y, z and then the angle, as VRML gives them. */
		static NEWMAT::Matrix vrmlRotationMatrix( QList< float > const &rotation );

		/** The matrix of the Transform node, as VRML97 defines it. */
		static NEWMAT::Matrix transformNodeMatrix( TransformNode *transformNode );

		/** Loads fileName into scene. Throws SIG_VRMLFileError if it cannot be read or parsed. */
		static void loadScene( SceneGraph &scene, QString const &fileName );

	private:

		static qsizetype indexAfterComment( QString const &vrml, qsizetype start );

		static qsizetype indexAfterString( QString const &vrml, qsizetype start );

		static qsizetype indexAfterWord( QString const &vrml, qsizetype start );

		static bool isPartOfWord( QChar c );
	};

}

#endif // SIGEL_ROBOT_SIG_VRMLUTILS_H
