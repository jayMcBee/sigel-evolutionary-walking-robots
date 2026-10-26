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
#include "SIGEL_GP/TST_SIG_GPExperiment.h"

#include "SIGEL_GP/SIG_GPExperimentClean.h"
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QtTest>

namespace {

	// The file text saveExperiment writes for experiment.
	QString savedText( SIGEL_GP::SIG_GPExperiment &experiment )
	{
		QString text;
		QTextStream stream( &text );
		experiment.saveExperiment( stream );
		stream.flush();
		return text;
	}

}

// Each block holds a value other than its default, so a block read by the wrong reader shows.
void SIGEL_GP::TST_SIG_GPExperiment::loadExperimentReadsEachBlockThatSaveExperimentWrote()
{
	SIG_GPExperiment written;
	written.simulationParameter.setStepSize( 0.02 );
	written.environment.setVeloDamping( 0.5 );
	written.gpParameter.setMinIndLength( 123 );
	written.population.setPoolGeneration( 9 );
	written.experimentHistory.append( new SIG_GPExperimentHistoryEntry( 4, QDateTime( QDate( 2026, 10, 26 ), QTime( 12, 0 ) ), 2.5, 0.5, 1.5 ) );
	written.comment = "a comment\n";
	QString text = savedText( written );
	QTextStream stream( &text );
	SIG_GPExperiment read;
	QString thrown;

	try {
		read.loadExperiment( stream );
	}
	catch ( SIGEL_Tools::SIG_Exception const &e ) {
		thrown = e.getMessage();
	}

	QCOMPARE( thrown, QString() );
	QCOMPARE( read.simulationParameter.getStepSize(), 0.02 );
	QCOMPARE( read.environment.getVeloDamping(), 0.5 );
	QCOMPARE( read.gpParameter.getMinIndLength(), 123 );
	QCOMPARE( read.population.getPoolGeneration(), 9 );
	QCOMPARE( read.experimentHistory.size(), 1 );
	QCOMPARE( read.experimentHistory.value( 0 )->getGenerationNo(), 4 );
	QCOMPARE( read.experimentHistory.value( 0 )->getMaxFitness(), 2.5 );
	QCOMPARE( read.comment, QString( "a comment\n" ) );
	QCOMPARE( savedText( read ), text );
}

void SIGEL_GP::TST_SIG_GPExperiment::loadExperimentThrowsForAFileThatEndsBeforeASeparator()
{
	QString text( "no separator\n" );
	QTextStream stream( &text );
	SIG_GPExperiment experiment;
	QString thrown;

	try {
		experiment.loadExperiment( stream );
	}
	catch ( SIGEL_Tools::SIG_Exception const &e ) {
		thrown = e.getMessage();
	}

	QVERIFY2( thrown.startsWith( "The file ends before a '#####' separator. It is not a complete experiment file.\n" ), qPrintable( thrown ) );
}
