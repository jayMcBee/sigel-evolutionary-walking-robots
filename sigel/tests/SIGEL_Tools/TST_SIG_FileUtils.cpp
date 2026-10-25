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
#include "SIGEL_Tools/TST_SIG_FileUtils.h"

#include "SIGEL_Tools/SIG_FileUtils.h"

#include <QtTest>

void SIGEL_Tools::TST_SIG_FileUtils::quotedIfContainsWhitespaceLeavesTextWithoutWhitespaceUnchanged()
{
	QCOMPARE( SIG_FileUtils::quotedIfContainsWhitespace( "robots/hammer/" ), QString( "robots/hammer/" ) );
	QCOMPARE( SIG_FileUtils::quotedIfContainsWhitespace( "" ), QString( "" ) );
}

void SIGEL_Tools::TST_SIG_FileUtils::quotedIfContainsWhitespaceQuotesTextWithASpaceATabOrALineBreak()
{
	QCOMPARE( SIG_FileUtils::quotedIfContainsWhitespace( "My Robots/" ), QString( "\"My Robots/\"" ) );
	QCOMPARE( SIG_FileUtils::quotedIfContainsWhitespace( "a\tb" ), QString( "\"a\tb\"" ) );
	QCOMPARE( SIG_FileUtils::quotedIfContainsWhitespace( "a\nb" ), QString( "\"a\nb\"" ) );
}

void SIGEL_Tools::TST_SIG_FileUtils::readWordOrQuotedReadsUpToTheNextWhitespace()
{
	QString text( "  first second\nthird" );
	QTextStream stream( &text, QIODevice::ReadOnly );

	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "first" ) );
	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "second" ) );
	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "third" ) );
}

void SIGEL_Tools::TST_SIG_FileUtils::readWordOrQuotedReadsAQuotedValueWithItsWhitespace()
{
	QString text( "\"My Robots/\" y" );
	QTextStream stream( &text, QIODevice::ReadOnly );

	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "My Robots/" ) );
	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "y" ) );
}

void SIGEL_Tools::TST_SIG_FileUtils::readWordOrQuotedReadsBackWhatQuotedIfContainsWhitespaceWrote()
{
	QStringList const values = { "plain", "with space", "with\ttab", " leading", "trailing " };

	QString text;
	QTextStream out( &text );
	for ( QString const &value : values )
		out << SIG_FileUtils::quotedIfContainsWhitespace( value ) << ' ';
	out.flush();

	QTextStream in( &text, QIODevice::ReadOnly );
	for ( QString const &value : values )
		QCOMPARE( SIG_FileUtils::readWordOrQuoted( in ), value );
}

void SIGEL_Tools::TST_SIG_FileUtils::readWordOrQuotedGivesAnEmptyWordAtTheEndOfTheText()
{
	QString text( "last  " );
	QTextStream stream( &text, QIODevice::ReadOnly );

	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString( "last" ) );
	QCOMPARE( SIG_FileUtils::readWordOrQuoted( stream ), QString() );
}
