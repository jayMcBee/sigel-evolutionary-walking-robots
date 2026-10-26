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
#include "SIGEL_GP/TST_SIG_GPPVMData.h"

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_GP/SIG_GPPVMData.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QtTest>

namespace {

	// The text SIG_Program writes for program.
	QString programText( SIGEL_Program::SIG_Program const &program )
	{
		QString text;
		QTextStream stream( &text );
		program.writeToFile( stream );
		stream.flush();
		return text;
	}

	// The message a master sends for a program of two lines.
	QString savedMessage()
	{
		SIGEL_Robot::SIG_Robot robot;
		SIGEL_Environment::SIG_Environment environment;
		SIGEL_Simulation::SIG_SimulationParameters simulationParameters;
		SIGEL_GP::SIG_GPPVMData data( robot, environment, simulationParameters, "fitness id", true );
		data.setActGeneration( 7 );
		data.setFitnessResetInterval( 3 );
		data.setExperimentName( "my experiment" );
		data.setIndividualName( "individual 42" );
		data.setIndividualFitness( 1.25 );
		QString programSource( "ADD 1,0\nADD 2,-1\n" );
		QTextStream programStream( &programSource );
		SIGEL_Program::SIG_Program program;
		program.readFromFile( programStream );
		QString text;
		QTextStream stream( &text );

		data.savePVMDataTransfer( stream, program );
		stream.flush();

		return text;
	}

}

void SIGEL_GP::TST_SIG_GPPVMData::loadPVMDataTransferReadsTheSettingsAndTheProgramThatSavePVMDataTransferWrote()
{
	QString text = savedMessage();
	QTextStream stream( &text );
	SIGEL_Robot::SIG_Robot robot;
	SIGEL_Environment::SIG_Environment environment;
	SIGEL_Simulation::SIG_SimulationParameters simulationParameters;
	SIG_GPPVMData data( robot, environment, simulationParameters, "", false );
	SIGEL_Program::SIG_Program program;
	QString thrown;

	try {
		data.loadPVMDataTransfer( stream, program );
	}
	catch ( SIGEL_Tools::SIG_Exception const &e ) {
		thrown = e.getMessage();
	}

	QCOMPARE( thrown, QString() );
	QCOMPARE( data.getFitnessFunctionName(), QString( "fitness id" ) );
	QCOMPARE( data.getActGeneration(), 7 );
	QCOMPARE( data.getFitnessResetInterval(), 3 );
	QCOMPARE( data.getVisualize(), true );
	QCOMPARE( data.getExperimentName(), QString( "my experiment" ) );
	QCOMPARE( data.getIndividualName(), QString( "individual 42" ) );
	QCOMPARE( data.getIndividualFitness(), 1.25 );
	QCOMPARE( programText( program ), QString( "       ADD 1,0\n       ADD 2,-1\n" ) );
}

void SIGEL_GP::TST_SIG_GPPVMData::loadPVMDataTransferThrowsForAMessageThatEndsBeforeItsLastBlock()
{
	QString text = savedMessage();
	QVERIFY( text.endsWith( "\n#####\n" ) );
	text.chop( QString( "#####\n" ).size() );
	QTextStream stream( &text );
	SIGEL_Robot::SIG_Robot robot;
	SIGEL_Environment::SIG_Environment environment;
	SIGEL_Simulation::SIG_SimulationParameters simulationParameters;
	SIG_GPPVMData data( robot, environment, simulationParameters, "", false );
	SIGEL_Program::SIG_Program program;
	QString thrown;

	try {
		data.loadPVMDataTransfer( stream, program );
	}
	catch ( SIGEL_Tools::SIG_Exception const &e ) {
		thrown = e.getMessage();
	}

	QVERIFY2( thrown.startsWith( "The message from the master ends before its last block. Are the master and this slave the same build?\n" ), qPrintable( thrown ) );
}
