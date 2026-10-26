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
#include "SIGEL_GP/TST_SIG_GPPVMHost.h"

#include "SIGEL_GP/SIG_GPPVMHost.h"

#include <QtTest>

void SIGEL_GP::TST_SIG_GPPVMHost::printWritesTheFieldsWithTheDirectoryQuoted()
{
	SIG_GPPVMHost const host( "node1", 4, true, QDir( "/opt/sigel" ) );

	QCOMPARE( host.print(), QString( "node1 4 1 \"/opt/sigel\"\n" ) );
}

void SIGEL_GP::TST_SIG_GPPVMHost::constructorReadsWhatPrintWrote_data()
{
	QTest::addColumn< QString >( "name" );
	QTest::addColumn< int >( "maxSlaves" );
	QTest::addColumn< bool >( "enabled" );
	QTest::addColumn< QString >( "directory" );

	QTest::newRow( "enabled" ) << "node1" << 4 << true << "/opt/sigel";
	QTest::newRow( "disabled" ) << "node2" << 3 << false << "/opt/sigel";
	QTest::newRow( "default directory" ) << "." << 1 << true << QString();
	QTest::newRow( "directory with a space" ) << "node1" << 2 << true << "/tmp/my slaves";
}

void SIGEL_GP::TST_SIG_GPPVMHost::constructorReadsWhatPrintWrote()
{
	QFETCH( QString, name );
	QFETCH( int, maxSlaves );
	QFETCH( bool, enabled );
	QFETCH( QString, directory );
	SIG_GPPVMHost const written( name, maxSlaves, enabled, QDir( directory ) );

	SIG_GPPVMHost const read( written.print() );

	QCOMPARE( read.name, name );
	QCOMPARE( read.maxSlaves, maxSlaves );
	QCOMPARE( read.enabled, enabled );
	QCOMPARE( read.executableDir.path(), written.executableDir.path() );
}

void SIGEL_GP::TST_SIG_GPPVMHost::constructorKeepsTheWhitespaceInTheDirectory()
{
	SIG_GPPVMHost const read( "node1 4 1 \"/tmp/my slaves\tbin\"\n" );

	QCOMPARE( read.executableDir.path(), QString( "/tmp/my slaves\tbin" ) );
}

void SIGEL_GP::TST_SIG_GPPVMHost::constructorKeepsNonAsciiCharactersInTheDirectory()
{
	SIG_GPPVMHost const read( QString::fromUtf8( "node1 4 1 \"/home/łukasz/€\"\n" ) );

	QCOMPARE( read.executableDir.path(), QString::fromUtf8( "/home/łukasz/€" ) );
}
