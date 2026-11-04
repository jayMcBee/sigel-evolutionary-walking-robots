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
#include "MT_GUI/TST_MT_SelectionWidget.h"

#include "MT_Control/MT_EstimationState.h"
#include "MT_Control/MT_EstimationStrategy.h"
#include "MT_GPSystem/MT_ErrorCode.h"
#include "MT_GPSystem/MT_GPManager.h"
#include "MT_GUI/MT_SelectionWidget.h"

#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QTextStream>
#include <QtTest>

const char *const TST_MT_SelectionWidget::gpSetup = R"(Generation:
0

Randomizer:
10
100
10
50
500
750
1000
500
500
400
700
1000
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
10000
11000
12000
13000
14000
15000
16000
17000

Constant:
newConstant
30

Statistics:
0

OffspringSize:
%1


Population:
%2

BrutSize:
2

TournamentManager:
1
%3

FitnessTranier:
1
0
20
250
10

Trainingset:
0
20
0
)";

void TST_MT_SelectionWidget::initTestCase()
{
	previousPlatform = qgetenv( "QT_QPA_PLATFORM" );
	qputenv( "QT_QPA_PLATFORM", "offscreen" );
	application = std::make_unique<QApplication>( applicationArgumentCount, applicationArguments );
}

void TST_MT_SelectionWidget::cleanupTestCase()
{
	application.reset();
	if( previousPlatform.isNull() )
	{
		qunsetenv( "QT_QPA_PLATFORM" );
		return;
	}

	qputenv( "QT_QPA_PLATFORM", previousPlatform );
}

void TST_MT_SelectionWidget::tournamentSizeListHoldsExactlyTheValidSizesAscending_data()
{
	QTest::addColumn<int>( "parentSize" );
	QTest::addColumn<int>( "offspringSize" );
	QTest::addColumn<QStringList>( "sizes" );

	QTest::newRow( "parents 10, offspring 10" ) << 10 << 10 << QStringList { "1", "2", "5", "10" };
	QTest::newRow( "parents 10, offspring 20" ) << 10 << 20 << QStringList { "2", "4", "10", "20" };
	QTest::newRow( "parents 6, offspring 18" ) << 6 << 18 << QStringList { "3", "6", "9", "18" };
	QTest::newRow( "parents 12, offspring 36" ) << 12 << 36 << QStringList { "3", "6", "9", "12", "18", "36" };
	QTest::newRow( "parents 1, offspring 5" ) << 1 << 5 << QStringList { "5" };
}

void TST_MT_SelectionWidget::tournamentSizeListHoldsExactlyTheValidSizesAscending()
{
	QFETCH( int, parentSize );
	QFETCH( int, offspringSize );
	QFETCH( QStringList, sizes );

	MT_SelectionWidget widget;
	widget.calculateTournamentSize( parentSize, offspringSize, offspringSize );

	QComboBox *tournamentSizes = widget.findChild<QComboBox *>( "tournamentSizeComboBox" );
	QVERIFY( tournamentSizes );
	QCOMPARE( listedSizes( *tournamentSizes ), sizes );
}

void TST_MT_SelectionWidget::tournamentSizeListOfASecondPairHoldsOnlyItsValidSizes_data()
{
	QTest::addColumn<int>( "firstParentSize" );
	QTest::addColumn<int>( "firstOffspringSize" );
	QTest::addColumn<int>( "parentSize" );
	QTest::addColumn<int>( "offspringSize" );
	QTest::addColumn<QStringList>( "sizes" );

	QTest::newRow( "longer list, then shorter" ) << 12 << 36 << 10 << 10 << QStringList { "1", "2", "5", "10" };
	QTest::newRow( "shorter list, then longer" ) << 1 << 5 << 12 << 36 << QStringList { "3", "6", "9", "12", "18", "36" };
	QTest::newRow( "same parents, more offspring" ) << 10 << 10 << 10 << 20 << QStringList { "2", "4", "10", "20" };
}

void TST_MT_SelectionWidget::tournamentSizeListOfASecondPairHoldsOnlyItsValidSizes()
{
	QFETCH( int, firstParentSize );
	QFETCH( int, firstOffspringSize );
	QFETCH( int, parentSize );
	QFETCH( int, offspringSize );
	QFETCH( QStringList, sizes );

	MT_SelectionWidget widget;
	widget.calculateTournamentSize( firstParentSize, firstOffspringSize, firstOffspringSize );
	widget.calculateTournamentSize( parentSize, offspringSize, offspringSize );

	QComboBox *tournamentSizes = widget.findChild<QComboBox *>( "tournamentSizeComboBox" );
	QVERIFY( tournamentSizes );
	QCOMPARE( listedSizes( *tournamentSizes ), sizes );
}

void TST_MT_SelectionWidget::tournamentSizeIsKeptOrMovesToTheNearestValidSize_data()
{
	QTest::addColumn<int>( "parentSize" );
	QTest::addColumn<int>( "offspringSize" );
	QTest::addColumn<int>( "oldTournamentSize" );
	QTest::addColumn<int>( "tournamentSize" );

	QTest::newRow( "valid 4 is kept" ) << 10 << 20 << 4 << 4;
	QTest::newRow( "valid 5 is kept" ) << 10 << 10 << 5 << 5;
	QTest::newRow( "valid 36 is kept" ) << 12 << 36 << 36 << 36;
	QTest::newRow( "5 moves down to 4" ) << 10 << 20 << 5 << 4;
	QTest::newRow( "8 moves up to 10" ) << 10 << 20 << 8 << 10;
	QTest::newRow( "17 moves up to 20" ) << 10 << 20 << 17 << 20;
	QTest::newRow( "20 moves down to the largest, 10" ) << 10 << 10 << 20 << 10;
	QTest::newRow( "1 moves up to the smallest, 3" ) << 6 << 18 << 1 << 3;
	QTest::newRow( "only 5 is valid" ) << 1 << 5 << 1 << 5;
}

void TST_MT_SelectionWidget::tournamentSizeIsKeptOrMovesToTheNearestValidSize()
{
	QFETCH( int, parentSize );
	QFETCH( int, offspringSize );
	QFETCH( int, oldTournamentSize );
	QFETCH( int, tournamentSize );

	MT_SelectionWidget widget;
	QCOMPARE( widget.calculateTournamentSize( parentSize, offspringSize, oldTournamentSize ), tournamentSize );
}

void TST_MT_SelectionWidget::tournamentSizeHalfwayBetweenTwoValidSizesMovesToOneOfThem_data()
{
	QTest::addColumn<int>( "parentSize" );
	QTest::addColumn<int>( "offspringSize" );
	QTest::addColumn<int>( "oldTournamentSize" );
	QTest::addColumn<int>( "smallerSize" );
	QTest::addColumn<int>( "largerSize" );

	QTest::newRow( "3 between 2 and 4" ) << 10 << 20 << 3 << 2 << 4;
	QTest::newRow( "15 between 12 and 18" ) << 12 << 36 << 15 << 12 << 18;
}

void TST_MT_SelectionWidget::tournamentSizeHalfwayBetweenTwoValidSizesMovesToOneOfThem()
{
	QFETCH( int, parentSize );
	QFETCH( int, offspringSize );
	QFETCH( int, oldTournamentSize );
	QFETCH( int, smallerSize );
	QFETCH( int, largerSize );

	MT_SelectionWidget widget;
	const int tournamentSize = widget.calculateTournamentSize( parentSize, offspringSize, oldTournamentSize );
	QVERIFY2( tournamentSize == smallerSize || tournamentSize == largerSize, qPrintable( QString::number( tournamentSize ) ) );
}

void TST_MT_SelectionWidget::overproductionChangeShowsTheNewSizesAndCounts_data()
{
	QTest::addColumn<int>( "parentSize" );
	QTest::addColumn<int>( "factor" );
	QTest::addColumn<int>( "oldTournamentSize" );
	QTest::addColumn<int>( "newFactor" );
	QTest::addColumn<QStringList>( "sizes" );
	QTest::addColumn<int>( "tournamentSize" );
	QTest::addColumn<int>( "pool" );
	QTest::addColumn<int>( "count" );
	QTest::addColumn<int>( "winnerCount" );

	QTest::newRow( "valid 10 is kept" ) << 10 << 1 << 10 << 2 << QStringList { "2", "4", "10", "20" } << 10 << 20 << 2 << 5;
	QTest::newRow( "5 moves down to 4" ) << 10 << 1 << 5 << 2 << QStringList { "2", "4", "10", "20" } << 4 << 20 << 5 << 2;
	QTest::newRow( "10 moves down to 6" ) << 10 << 2 << 10 << 3 << QStringList { "3", "6", "15", "30" } << 6 << 30 << 5 << 2;
	QTest::newRow( "20 moves down to 15" ) << 10 << 2 << 20 << 3 << QStringList { "3", "6", "15", "30" } << 15 << 30 << 2 << 5;
	QTest::newRow( "20 moves down to the largest, 10" ) << 10 << 2 << 20 << 1 << QStringList { "1", "2", "5", "10" } << 10 << 10 << 1 << 10;
	QTest::newRow( "8 moves up to 9" ) << 12 << 2 << 8 << 3 << QStringList { "3", "6", "9", "12", "18", "36" } << 9 << 36 << 4 << 3;
}

void TST_MT_SelectionWidget::overproductionChangeShowsTheNewSizesAndCounts()
{
	QFETCH( int, parentSize );
	QFETCH( int, factor );
	QFETCH( int, oldTournamentSize );
	QFETCH( int, newFactor );
	QFETCH( QStringList, sizes );
	QFETCH( int, tournamentSize );
	QFETCH( int, pool );
	QFETCH( int, count );
	QFETCH( int, winnerCount );

	std::unique_ptr<MT_GPManager> gpManager = manager( parentSize, parentSize * factor, oldTournamentSize );
	QCOMPARE( gpManager->getLastError(), MT_ErrorCode::None );
	MT_EstimationState state {};
	state.strategy = MT_EstimationStrategy::MeanError;

	MT_SelectionWidget widget;
	widget.onShow( gpManager.get(), &state );

	QSpinBox *overproduction = widget.findChild<QSpinBox *>( "overProductionSpinBox" );
	QComboBox *tournamentSizes = widget.findChild<QComboBox *>( "tournamentSizeComboBox" );
	QLabel *poolSize = widget.findChild<QLabel *>( "poolSizeLabel" );
	QLabel *tournamentCount = widget.findChild<QLabel *>( "tournamentCountLabel" );
	QLabel *winners = widget.findChild<QLabel *>( "winNumLabel" );
	QVERIFY( overproduction );
	QVERIFY( tournamentSizes );
	QVERIFY( poolSize );
	QVERIFY( tournamentCount );
	QVERIFY( winners );
	QCOMPARE( overproduction->value(), factor );
	QCOMPARE( tournamentSizes->currentText(), QString::number( oldTournamentSize ) );

	overproduction->setValue( newFactor );

	QCOMPARE( listedSizes( *tournamentSizes ), sizes );
	QCOMPARE( tournamentSizes->currentText(), QString::number( tournamentSize ) );
	QCOMPARE( poolSize->text(), QString::number( pool ) );
	QCOMPARE( tournamentCount->text(), QString::number( count ) );
	QCOMPARE( winners->text(), QString::number( winnerCount ) );
}

std::unique_ptr<MT_GPManager> TST_MT_SelectionWidget::manager( int parentSize, int offspringSize, int tournamentSize )
{
	QString text = QString( gpSetup ).arg( offspringSize ).arg( parentSize ).arg( tournamentSize );
	QTextStream stream( &text, QIODevice::ReadOnly );
	return std::make_unique<MT_GPManager>( stream );
}

QStringList TST_MT_SelectionWidget::listedSizes( const QComboBox &tournamentSizes )
{
	QStringList listed;
	for( int index = 0; index < tournamentSizes.count(); index++ )
	{
		listed << tournamentSizes.itemText( index );
	}
	return listed;
}
