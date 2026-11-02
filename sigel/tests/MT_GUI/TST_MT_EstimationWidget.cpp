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
#include "MT_GUI/TST_MT_EstimationWidget.h"

#include "MT_Control/MT_EstimationState.h"
#include "MT_GUI/MT_EstimationWidget.h"

#include <QComboBox>
#include <QGroupBox>
#include <QtTest>

void TST_MT_EstimationWidget::initTestCase()
{
	previousPlatform = qgetenv( "QT_QPA_PLATFORM" );
	qputenv( "QT_QPA_PLATFORM", "offscreen" );
	application = std::make_unique<QApplication>( applicationArgumentCount, applicationArguments );
}

void TST_MT_EstimationWidget::cleanupTestCase()
{
	application.reset();
	if( previousPlatform.isNull() )
	{
		qunsetenv( "QT_QPA_PLATFORM" );
		return;
	}

	qputenv( "QT_QPA_PLATFORM", previousPlatform );
}

void TST_MT_EstimationWidget::eachStrategyShowsItsNameAndIsStoredUnchanged_data()
{
	QTest::addColumn<int>( "strategy" );
	QTest::addColumn<QString>( "name" );
	QTest::addColumn<QString>( "title" );

	QTest::newRow( "1" ) << 1 << "Mean error" << "Evaluation";
	QTest::newRow( "2" ) << 2 << "Cleaned mean error" << "Evaluation";
	QTest::newRow( "9" ) << 9 << "Adaptive error" << "Evaluation";
	QTest::newRow( "10" ) << 10 << "Adaptive cleaned error" << "Evaluation";
	QTest::newRow( "13" ) << 13 << "Don't use MetaGP" << "Evaluation";
	QTest::newRow( "3" ) << 3 << "Error percentage" << "Classification";
	QTest::newRow( "4" ) << 4 << "Adaptive - generation" << "Classification";
	QTest::newRow( "5" ) << 5 << "Adaptive - fitness" << "Classification";
	QTest::newRow( "6" ) << 6 << "Save the best" << "Classification";
	QTest::newRow( "8" ) << 8 << "Don't use MetaGP" << "Classification";
}

void TST_MT_EstimationWidget::eachStrategyShowsItsNameAndIsStoredUnchanged()
{
	QFETCH( int, strategy );
	QFETCH( QString, name );
	QFETCH( QString, title );

	MT_EstimationState state {};
	state.strategy = static_cast<MT_EstimationStrategy>( strategy );
	state.tolerance = 10;
	state.refreshInterval = 5;

	MT_EstimationWidget widget;
	widget.onShow( nullptr, &state );

	QComboBox *strategies = widget.findChild<QComboBox *>( "strategyComboBox" );
	QGroupBox *group = widget.findChild<QGroupBox *>( "estimationGroupBox" );
	QVERIFY( strategies );
	QVERIFY( group );
	QCOMPARE( strategies->currentText(), name );
	QCOMPARE( group->title(), title );

	// 0 is no strategy, so the value can only come from onHide.
	state.strategy = static_cast<MT_EstimationStrategy>( 0 );
	QVERIFY( widget.onHide( nullptr, &state ) );
	QCOMPARE( static_cast<int>( state.strategy ), strategy );
}
