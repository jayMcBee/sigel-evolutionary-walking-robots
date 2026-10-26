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
#ifndef SIGEL_TOOLS_SIG_FILEUTILS_H
#define SIGEL_TOOLS_SIG_FILEUTILS_H

#include <QString>
#include <QTextStream>

namespace SIGEL_Tools
{

	/**
	 * Helpers for reading and writing SIGEL's text files, whose values are
	 * separated by whitespace.
	 */
	class SIG_FileUtils
	{
	public:

		/**
		 * The text in double quotes if it contains whitespace, else unchanged,
		 * so that it stays one value. Empty text and text with a double quote in
		 * it do not survive.
		 */
		static QString quotedIfContainsWhitespace( QString const &text );

		/**
		 * Reads one value that quotedIfContainsWhitespace wrote: up to the next
		 * whitespace, or, after a double quote, up to the closing one.
		 */
		static QString readWordOrQuoted( QTextStream &stream );

		/**
		 * Reads the lines up to the next line that is exactly endLine, and that
		 * line. Gives them each followed by a newline, without endLine. Throws a
		 * SIG_Exception with missingEndMessage if the text ends first.
		 */
		static QString readLinesUpTo( QTextStream &stream, QString const &endLine, QString const &missingEndMessage );

	private:

		static bool containsWhitespace( QString const &text );
	};

}

#endif // SIGEL_TOOLS_SIG_FILEUTILS_H
