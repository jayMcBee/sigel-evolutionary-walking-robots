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
#ifndef SIGEL_MASTERGUI_TST_SIG_GPPARAMETER_H
#define SIGEL_MASTERGUI_TST_SIG_GPPARAMETER_H

#include <QApplication>
#include <QByteArray>
#include <QObject>

#include <memory>

namespace SIGEL_MasterGUI
{

	/**
	 * The unit tests of SIG_GPParameter, the GP Parameters page.
	 *
	 * The class makes its own QApplication on the offscreen platform, because
	 * sigel/tests/main.cpp has none.
	 */
	class TST_SIG_GPParameter : public QObject
	{
		Q_OBJECT

	private slots:

		void initTestCase();

		void cleanupTestCase();

		void maximumLengthCannotBeSetBelowTheMinimumLength();

		void minimumLengthAboveTheMaximumLengthRaisesTheMaximumLength();

	private:

		std::unique_ptr<QApplication> application;
		int applicationArgumentCount = 1;
		char applicationName[10] = "sigeltest";

		// QApplication keeps these for its whole life.
		char *applicationArguments[2] = { applicationName, nullptr };
		QByteArray previousPlatform;
	};

}

#endif // SIGEL_MASTERGUI_TST_SIG_GPPARAMETER_H
