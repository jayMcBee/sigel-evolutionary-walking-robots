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
#include "SIGEL_Robot/TST_SIG_VRMLUtils.h"

#include "SIGEL_Robot/SIG_VRMLUtils.h"

#include <QtTest>

#include <cmath>

namespace
{

	double const epsilon = 1e-5;

	// Empty if actual is expected, else where it differs first. Compared at the
	// call site, so a failure names the line of that call and stops the test there.
	QString matrixMismatch( NEWMAT::Matrix actual, double const ( &expected )[ 4 ][ 4 ] )
	{
		if ( actual.Nrows() != 4 || actual.Ncols() != 4 )
			return QString( "is %1x%2, expected 4x4" ).arg( actual.Nrows() ).arg( actual.Ncols() );
		for ( int r = 0; r < 4; r++ )
			for ( int c = 0; c < 4; c++ )
				if ( !( std::fabs( actual( r + 1, c + 1 ) - expected[ r ][ c ] ) < epsilon ) )
					return QString( "(%1,%2) is %3, expected %4" )
						.arg( r + 1 ).arg( c + 1 ).arg( actual( r + 1, c + 1 ) ).arg( expected[ r ][ c ] );
		return QString();
	}

}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesTheLineOfAUseDirectlyAfterANodeEnd()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { } USE S\n" ), 2 );
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { }USE S\n" ), 2 );
}

// vrml.l has no rule for a form feed, vertical tab or carriage return, so
// flex's default rule passes them over without a token and without counting
// a line; DOS line endings do not separate the USE either and count as one
// line each.
void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndFindsAUseThatWhitespaceOtherThanASpaceSeparatesFromANodeEnd()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { }\t\f\v\r\n\r\nUSE S\n" ), 4 );
}

// A comment between the "}" and the USE does not separate them for the parser.
void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndCountsTheLinesInStringsAndComments()
{
	QString const vrml( "# a } comment\n"
	                    "DEF S Group { } ,\n"
	                    "WorldInfo { info [ \"a\nb\" ] } # c\n"
	                    "\n"
	                    "USE S\n" );

	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( vrml ), 6 );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesTheLineOfTheFirstSuchUse()
{
	QString const vrml( "DEF S Group { }\n"
	                    "DEF R Group { }\n"
	                    "Group { } USE S\n"
	                    "Group { } USE R\n" );

	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( vrml ), 3 );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroForAUseInACommentOrAString()
{
	QString const vrml( "Group { }\n"
	                    "# } USE S\n"
	                    "WorldInfo { title \"} USE S\" }\n" );

	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( vrml ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroForTextThatEndsInACommentWithoutANewline()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "Group { } # end" ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroWhenACommaSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "DEF S Group { }\nGroup { } ,\nUSE S\n" ), 0 );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroWhenAFieldNameSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "Shape { appearance Appearance { } geometry USE G }\n" ), 0 );
}

// Not valid VRML, which has no string directly before a USE: it only shows
// that a string ends what follows a "}".
void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroWhenAStringSeparatesAUseFromANodeEnd()
{
	QCOMPARE( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "Group { } \"x\" USE S\n" ), 0 );
}

// CyberVRML97 lexes each as one name, the USE keyword being case-sensitive (vrml.l).
void SIGEL_Robot::TST_SIG_VRMLUtils::lineOfUseAfterNodeEndGivesZeroForAWordOtherThanUseDirectlyAfterANodeEnd()
{
	QStringList const names = { "USEFUL", "USE1", "USE_FUL", "USE-FUL", "USE+FUL", "use", "Use" };
	for ( QString const &name : names )
		QVERIFY2( SIG_VRMLUtils::lineOfUseAfterNodeEnd( "Group { } " + name + " { }\n" ) == 0, qPrintable( name ) );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::inverseVRMLRotationMatrixPinsTheCurrentSignOfTheSineTerms()
{
	// Pins the current behaviour: the sine terms have the opposite sign from
	// rotationMatrix in IFunctions.cpp, and whether that is right is open (item 144).
	double const quarter = M_PI / 2;
	double const aboutX[ 4 ][ 4 ] = { { 1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, -1, 0, 0 }, { 0, 0, 0, 1 } };
	double const aboutY[ 4 ][ 4 ] = { { 0, 0, -1, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 }, { 0, 0, 0, 1 } };
	double const aboutZ[ 4 ][ 4 ] = { { 0, 1, 0, 0 }, { -1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

	QCOMPARE( matrixMismatch( SIG_VRMLUtils::inverseVRMLRotationMatrix( { 1, 0, 0, float( quarter ) } ), aboutX ), QString() );
	QCOMPARE( matrixMismatch( SIG_VRMLUtils::inverseVRMLRotationMatrix( { 0, 1, 0, float( quarter ) } ), aboutY ), QString() );
	QCOMPARE( matrixMismatch( SIG_VRMLUtils::inverseVRMLRotationMatrix( { 0, 0, 1, float( quarter ) } ), aboutZ ), QString() );
}

void SIGEL_Robot::TST_SIG_VRMLUtils::inverseVRMLRotationMatrixTurnsHalfwayAboutAnOffAxisDirection()
{
	// A half turn has no sine terms, so this holds for either sign: 2 a a^T - I.
	// No axis component is zero, so every product of two of them counts.
	double const expected[ 4 ][ 4 ] = { { -0.5392, 0.576, 0.6144, 0 },
	                                    { 0.576, -0.28, 0.768, 0 },
	                                    { 0.6144, 0.768, -0.1808, 0 },
	                                    { 0, 0, 0, 1 } };

	QCOMPARE( matrixMismatch( SIG_VRMLUtils::inverseVRMLRotationMatrix( { 0.48f, 0.6f, 0.64f, float( M_PI ) } ), expected ), QString() );
}
