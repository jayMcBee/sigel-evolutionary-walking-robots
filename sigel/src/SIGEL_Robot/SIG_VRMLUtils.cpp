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
#include "SIGEL_Robot/SIG_VRMLUtils.h"

#include <cmath>

namespace SIGEL_Robot
{

	int SIG_VRMLUtils::lineOfUseAfterNodeEnd( QString const &vrml )
	{
		int line = 1;
		bool afterNodeEnd = false;
		qsizetype i = 0;

		// The text splits into tokens as in CyberVRML97's vrml.l.
		while (i < vrml.size())
		{
			QChar const c = vrml.at( i );
			if (c == '#')
			{
				i = indexAfterComment( vrml, i );
				continue;
			}

			if (c == '"')
			{
				qsizetype const end = indexAfterString( vrml, i );
				line += QStringView( vrml ).mid( i, end - i ).count( u'\n' );
				i = end;
				afterNodeEnd = false;
				continue;
			}

			if (isPartOfWord( c ))
			{
				qsizetype const end = indexAfterWord( vrml, i );
				if (afterNodeEnd && QStringView( vrml ).mid( i, end - i ) == u"USE")
					return line;

				i = end;
				afterNodeEnd = false;
				continue;
			}

			if (c == '\n')
				line++;
			else if (!c.isSpace())
				afterNodeEnd = (c == '}');

			i++;
		}
		return 0;
	}

	NEWMAT::Matrix SIG_VRMLUtils::inverseVRMLRotationMatrix( QList< float > const &rotation )
	{
		NEWMAT::Matrix rotationMatrix(4,4);
		double const x = rotation[0];
		double const y = rotation[1];
		double const z = rotation[2];
		double const a = rotation[3];

		double const sinA = std::sin(a);
		double const cosA = std::cos(a);
		double const t = 1 - cosA;

		rotationMatrix = 0;
		rotationMatrix(1,1) = (t * x * x) + cosA;
		rotationMatrix(1,2) = (t * x * y) + (sinA * z);
		rotationMatrix(1,3) = (t * x * z) - (sinA * y);
		rotationMatrix(2,1) = (t * x * y) - (sinA * z);
		rotationMatrix(2,2) = (t * y * y) + cosA;
		rotationMatrix(2,3) = (t * y * z) + (sinA * x);
		rotationMatrix(3,1) = (t * x * z) + (sinA * y);
		rotationMatrix(3,2) = (t * y * z) - (sinA * x);
		rotationMatrix(3,3) = (t * z * z) + cosA;
		rotationMatrix(4,4) = 1;

		return rotationMatrix;
	}

	qsizetype SIG_VRMLUtils::indexAfterComment( QString const &vrml, qsizetype start )
	{
		qsizetype const end = vrml.indexOf( u'\n', start );
		if (end < 0)
			return vrml.size();

		return end;
	}

	qsizetype SIG_VRMLUtils::indexAfterString( QString const &vrml, qsizetype start )
	{
		qsizetype const end = vrml.indexOf( u'"', start + 1 );
		if (end < 0)
			return vrml.size();

		return end + 1;
	}

	qsizetype SIG_VRMLUtils::indexAfterWord( QString const &vrml, qsizetype start )
	{
		qsizetype end = start;
		while (end < vrml.size() && isPartOfWord( vrml.at( end ) ))
			end++;
		return end;
	}

	bool SIG_VRMLUtils::isPartOfWord( QChar c )
	{
		return c.isLetterOrNumber() || c == '_' || c == '-' || c == '+' || c == '.';
	}

}
