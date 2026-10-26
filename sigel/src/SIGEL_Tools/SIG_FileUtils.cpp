/*
  Copyright 2026 Jan Barnholt

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
#include "SIGEL_Tools/SIG_FileUtils.h"

#include "SIGEL_Tools/SIG_Exception.h"

#include <algorithm>

namespace SIGEL_Tools
{

	QString SIG_FileUtils::quotedIfContainsWhitespace( QString const &text )
	{
		if (containsWhitespace( text ))
			return "\"" + text + "\"";

		return text;
	}

	QString SIG_FileUtils::readWordOrQuoted( QTextStream &stream )
	{
		QString word;
		QChar c;
		stream.skipWhiteSpace();
		stream >> c;
		if (c == '"')
		{
			stream >> c;
			while (stream.status() == QTextStream::Ok && c != '"')
			{
				word.append( c );
				stream >> c;
			}
		}
		else
		{
			while (stream.status() == QTextStream::Ok && !c.isSpace())
			{
				word.append( c );
				stream >> c;
			}
		}
		return word;
	}

	QString SIG_FileUtils::readLinesUpTo( QTextStream &stream, QString const &endLine, QString const &missingEndMessage )
	{
		QString lines;
		QString line;
		while ((line = stream.readLine()) != endLine)
		{
			if (line.isNull())
				throw SIG_Exception( __FILE__, __LINE__, missingEndMessage );

			lines += line + "\n";
		}
		return lines;
	}

	bool SIG_FileUtils::containsWhitespace( QString const &text )
	{
		return std::ranges::any_of( text, [] ( QChar const c )
		{
			return c.isSpace();
		} );
	}

}
