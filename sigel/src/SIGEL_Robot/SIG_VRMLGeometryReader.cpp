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
#include "SIGEL_Robot/SIG_VRMLGeometryReader.h"

namespace SIGEL_Robot
{

	int SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd (QString const & vrml)
	{
		int line = 1;
		bool afterNodeEnd = false;
		qsizetype i = 0;
		while (i < vrml.size ()) {
			QChar const c = vrml.at (i);
			if (c == '\n') {
				line++;
				i++;
			} else if (c.isSpace ())
				i++;
			else if (c == '#') {
				while (i < vrml.size () && vrml.at (i) != '\n')
					i++;
			} else if (c == '"') {
				for (i++; i < vrml.size () && vrml.at (i) != '"'; i++)
					if (vrml.at (i) == '\n')
						line++;
				i++;
				afterNodeEnd = false;
			} else if (c.isLetterOrNumber () || c == '_' || c == '-' || c == '+' || c == '.') {
				qsizetype const start = i;
				while (i < vrml.size ()
				       && (vrml.at (i).isLetterOrNumber () || vrml.at (i) == '_'
					   || vrml.at (i) == '-' || vrml.at (i) == '+' || vrml.at (i) == '.'))
					i++;
				if (afterNodeEnd && QStringView (vrml).mid (start, i - start) == u"USE")
					return line;
				afterNodeEnd = false;
			} else {
				afterNodeEnd = (c == '}');
				i++;
			}
		}
		return 0;
	}

}
