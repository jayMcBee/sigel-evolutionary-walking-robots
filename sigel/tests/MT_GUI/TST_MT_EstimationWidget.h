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
#ifndef MT_GUI_TST_MT_ESTIMATIONWIDGET_H
#define MT_GUI_TST_MT_ESTIMATIONWIDGET_H

#include <QApplication>
#include <QByteArray>
#include <QObject>

#include <memory>

/**
 * The unit tests of MT_EstimationWidget, the page of the MetaGP window that
 * shows the estimation strategy.
 *
 * The class makes its own QApplication on the offscreen platform, because
 * sigel/tests/main.cpp has none.
 */
class TST_MT_EstimationWidget : public QObject
{
	Q_OBJECT

private slots:

	void initTestCase();

	void cleanupTestCase();

	void eachStrategyShowsItsNameAndIsStoredUnchanged_data();

	void eachStrategyShowsItsNameAndIsStoredUnchanged();

private:

	std::unique_ptr<QApplication> application;
	int applicationArgumentCount = 1;
	char applicationName[10] = "sigeltest";

	// QApplication keeps these for its whole life.
	char *applicationArguments[2] = { applicationName, nullptr };
	QByteArray previousPlatform;
};

#endif // MT_GUI_TST_MT_ESTIMATIONWIDGET_H
