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
#include "SIGEL_MasterGUI/TST_SIG_GPParameter.h"

#include "SIGEL_GP/SIG_GPMetaGPExperiment.h"
#include "SIGEL_MasterGUI/SIG_GPParameter.h"

#include <QSpinBox>
#include <QtTest>

void SIGEL_MasterGUI::TST_SIG_GPParameter::initTestCase()
{
	previousPlatform = qgetenv( "QT_QPA_PLATFORM" );
	qputenv( "QT_QPA_PLATFORM", "offscreen" );
	application = std::make_unique<QApplication>( applicationArgumentCount, applicationArguments );
}

void SIGEL_MasterGUI::TST_SIG_GPParameter::cleanupTestCase()
{
	application.reset();
	if( previousPlatform.isNull() )
	{
		qunsetenv( "QT_QPA_PLATFORM" );
		return;
	}
	qputenv( "QT_QPA_PLATFORM", previousPlatform );
}

void SIGEL_MasterGUI::TST_SIG_GPParameter::maximumLengthCannotBeSetBelowTheMinimumLength()
{
	SIGEL_GP::SIG_GPMetaGPExperiment experiment;
	SIG_GPParameter page( nullptr, Qt::WindowFlags(), experiment );
	page.spinboxMinIndLength->setValue( 50 );

	page.spinboxMaxIndLength->setValue( 20 );

	QCOMPARE( page.spinboxMaxIndLength->value(), 50 );
}

void SIGEL_MasterGUI::TST_SIG_GPParameter::minimumLengthAboveTheMaximumLengthRaisesTheMaximumLength()
{
	SIGEL_GP::SIG_GPMetaGPExperiment experiment;
	SIG_GPParameter page( nullptr, Qt::WindowFlags(), experiment );
	page.spinboxMinIndLength->setValue( 10 );
	page.spinboxMaxIndLength->setValue( 30 );

	page.spinboxMinIndLength->setValue( 40 );

	QCOMPARE( page.spinboxMaxIndLength->value(), 40 );
}
