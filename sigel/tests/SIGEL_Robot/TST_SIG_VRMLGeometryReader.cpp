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
#include "SIGEL_Robot/TST_SIG_VRMLGeometryReader.h"

#include "SIGEL_Robot/SIG_VRMLGeometryReader.h"

#include <QtTest>

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesTheLineOfAUseDirectlyAfterANodeEnd()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { } USE S\n" ), 2 );
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { }USE S\n" ), 2 );
}

// vrml.l has no rule for a form feed, vertical tab or carriage return, so
// flex's default rule passes them over without a token and without counting
// a line; DOS line endings do not separate the USE either and count as one
// line each.
void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndFindsAUseThatWhitespaceOtherThanASpaceSeparatesFromANodeEnd()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { }\t\f\v\r\n\r\nUSE S\n" ), 4 );
}

// A comment between the "}" and the USE does not separate them for the parser.
void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndCountsTheLinesInStringsAndComments()
{
	QString const vrml( "# a } comment\n"
	                    "DEF S Group { } ,\n"
	                    "WorldInfo { info [ \"a\nb\" ] } # c\n"
	                    "\n"
	                    "USE S\n" );

	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( vrml ), 6 );
}

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesTheLineOfTheFirstSuchUse()
{
	QString const vrml( "DEF S Group { }\n"
	                    "DEF R Group { }\n"
	                    "Group { } USE S\n"
	                    "Group { } USE R\n" );

	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( vrml ), 3 );
}

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroForAUseInACommentOrAString()
{
	QString const vrml( "Group { }\n"
	                    "# } USE S\n"
	                    "WorldInfo { title \"} USE S\" }\n" );

	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( vrml ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroForTextThatEndsInACommentWithoutANewline()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "Group { } # end" ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroWhenACommaSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { } ,\nUSE S\n" ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroWhenAFieldNameSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "Shape { appearance Appearance { } geometry USE G }\n" ), 0 );
}

// Not valid VRML, which has no string directly before a USE: it only shows
// that a string ends what follows a "}".
void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroWhenAStringSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "Group { } \"x\" USE S\n" ), 0 );
}

// CyberVRML97 lexes each as one name, the USE keyword being case-sensitive (vrml.l).
void SIGEL_Robot::TST_SIG_VRMLGeometryReader::lineOfUseAfterNodeEndGivesZeroForAWordOtherThanUseDirectlyAfterANodeEnd()
{
	QStringList const names = { "USEFUL", "USE1", "USE_FUL", "USE-FUL", "USE+FUL", "use", "Use" };
	for ( QString const &name : names )
		QVERIFY2( SIG_VRMLGeometryReader::lineOfUseAfterNodeEnd( "Group { } " + name + " { }\n" ) == 0, qPrintable( name ) );
}
