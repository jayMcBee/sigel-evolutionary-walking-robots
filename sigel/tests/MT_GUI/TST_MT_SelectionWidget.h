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
#ifndef MT_GUI_TST_MT_SELECTIONWIDGET_H
#define MT_GUI_TST_MT_SELECTIONWIDGET_H

#include <QApplication>
#include <QByteArray>
#include <QComboBox>
#include <QObject>
#include <QStringList>

#include <memory>

class MT_GPManager;

/**
 * The unit tests of MT_SelectionWidget, the page of the MetaGP window that
 * sets the tournament selection.
 *
 * A tournament size is valid when it divides the offspring size and the
 * tournament count, offspring size / tournament size, divides the parent size.
 *
 * updateTSizeList is private. The tests reach it through calculateTournamentSize
 * and slotOverProdChanged, which both rebuild the list with it.
 *
 * The class makes its own QApplication on the offscreen platform, because
 * sigel/tests/main.cpp has none.
 */
class TST_MT_SelectionWidget : public QObject
{
	Q_OBJECT

private slots:

	void initTestCase();

	void cleanupTestCase();

	void tournamentSizeListHoldsExactlyTheValidSizesAscending_data();

	void tournamentSizeListHoldsExactlyTheValidSizesAscending();

	void tournamentSizeListOfASecondPairHoldsOnlyItsValidSizes_data();

	void tournamentSizeListOfASecondPairHoldsOnlyItsValidSizes();

	void tournamentSizeIsKeptOrMovesToTheNearestValidSize_data();

	void tournamentSizeIsKeptOrMovesToTheNearestValidSize();

	void tournamentSizeHalfwayBetweenTwoValidSizesMovesToOneOfThem_data();

	void tournamentSizeHalfwayBetweenTwoValidSizesMovesToOneOfThem();

	void overproductionChangeShowsTheNewSizesAndCounts_data();

	void overproductionChangeShowsTheNewSizesAndCounts();

private:

	// The part of a MetaGP configuration file that MT_GPManager reads:
	// %1 is the offspring size, %2 the parent size, %3 the tournament size.
	static const char *const gpSetup;

	std::unique_ptr<QApplication> application;
	int applicationArgumentCount = 1;
	char applicationName[10] = "sigeltest";

	// QApplication keeps these for its whole life.
	char *applicationArguments[2] = { applicationName, nullptr };
	QByteArray previousPlatform;

	std::unique_ptr<MT_GPManager> manager( int parentSize, int offspringSize, int tournamentSize );

	QStringList listedSizes( const QComboBox &tournamentSizes );
};

#endif // MT_GUI_TST_MT_SELECTIONWIDGET_H
