/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

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
#include <QList>
#include "SIGEL_GP/SIG_GPOperations.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include <qstringlist.h>
#include <cmath>

/*------------------------------------------Crossover-----------------------------------------*/
QList<SIGEL_GP::SIG_GPIndividual *> SIGEL_GP::SIG_GPOperations::crossOver( SIGEL_GP::SIG_GPIndividual& winner1,
                                                                           int poolPos1,
                                                                           SIG_GPIndividual& winner2,
                                                                           int poolPos2,
                                                                           QString newName1,
                                                                           QString newName2,
                                                                           SIGEL_Tools::SIG_Randomizer& randomizer,
                                                                           SIGEL_GP::SIG_GPParameter& gpParameter,
                                                                           SIGEL_Robot::SIG_LanguageParameters &languageParameters)
{
	SIGEL_GP::SIG_GPIndividual *crossoverInd1 = new SIGEL_GP::SIG_GPIndividual( SIGEL_Program::SIG_Program(),
	                                                                            newName1,
	                                                                            "",
	                                                                            "",
	                                                                            QDateTime(),
	                                                                            poolPos1,
	                                                                            winner1.getHistory() );

	SIGEL_GP::SIG_GPIndividual *crossoverInd2 = new SIGEL_GP::SIG_GPIndividual( SIGEL_Program::SIG_Program(),
	                                                                            newName2,
	                                                                            "",
	                                                                            "",
	                                                                            QDateTime(),
	                                                                            poolPos2,
	                                                                            winner2.getHistory() );

	double fitness1 = winner1.getFitness();
	double fitness2 = winner2.getFitness();

	long const minLength = gpParameter.getMinIndLength();
	long const maxLength = gpParameter.getMaxIndLength();
	int        historyInfo;

	SIGEL_Program::SIG_Program &newProgram1 = crossoverInd1->getProgramVar();
	SIGEL_Program::SIG_Program &newProgram2 = crossoverInd2->getProgramVar();

	SIGEL_Program::SIG_Program &winnerProgram1 = winner1.getProgramVar();
	SIGEL_Program::SIG_Program &winnerProgram2 = winner2.getProgramVar();

	// get first crossover points
	long int crossPoint1 = randomizer.getRandomLong( winnerProgram1.getProgramLength() - 1 ) + 1;
	long int crossPoint2 = randomizer.getRandomLong( winnerProgram2.getProgramLength() - 1 ) + 1;

	// The two-point segment length is at most the cube root of the parent's length:
	// short segments make crossover less destructive.
	long const cubeRootOfLength1 = static_cast<long>( std::cbrt( winnerProgram1.getProgramLength() ) );
	long const cubeRootOfLength2 = static_cast<long>( std::cbrt( winnerProgram2.getProgramLength() ) );

	long int segLen1 = randomizer.getRandomLong( cubeRootOfLength1 ) + 1;
	if ((crossPoint1+segLen1) >= winnerProgram1.getProgramLength())
	{
		segLen1 = winnerProgram1.getProgramLength() - crossPoint1 - 1;
	}

	long int segLen2 = randomizer.getRandomLong( cubeRootOfLength2 ) + 1;
	if ((crossPoint2+segLen2) >= winnerProgram2.getProgramLength())
	{
		segLen2 = winnerProgram2.getProgramLength() - crossPoint2 - 1;
	}

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Crossover with minimum length " << minLength << " and maximum length " << maxLength << " (0: no maximum)." << Qt::endl;
#endif

	// check new program lengths if min. program length was requested
	if( minLength > 0 &&
	    ( ( crossPoint1 + winnerProgram2.getProgramLength() - crossPoint2 < minLength ) ||
	      ( crossPoint2 + winnerProgram1.getProgramLength() - crossPoint1 < minLength ) ) )
	{
		crossPoint1 = winnerProgram1.getProgramLength() / 2;
		if ((crossPoint1+segLen1) >= winnerProgram1.getProgramLength())
		{
			segLen1 = winnerProgram1.getProgramLength() - crossPoint1 - 1;
		}

		crossPoint2 = winnerProgram2.getProgramLength() / 2;
		if ((crossPoint2+segLen2) >= winnerProgram2.getProgramLength())
		{
			segLen2 = winnerProgram2.getProgramLength() - crossPoint2 - 1;
		}

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Crossover points moved to the middle: " << crossPoint1 << " and " << crossPoint2 << "; the children get " << crossPoint1 + winnerProgram2.getProgramLength() - crossPoint2 << " and " << crossPoint2 + winnerProgram1.getProgramLength() - crossPoint1 << " lines." << Qt::endl;
#endif
	}

	// randomly select any recombination type; currently the 2-point-c/o
	// has a chance of 3:2 to be selected !
	bool const twoPoint = randomizer.getRandomInt( 5 ) >= 2;

	if( twoPoint )
		twoPointCrossOver( winnerProgram1, winnerProgram2, crossPoint1, crossPoint2, segLen1, segLen2, newProgram1, newProgram2 );
	else
		onePointCrossOver( winnerProgram1, winnerProgram2, crossPoint1, crossPoint2, newProgram1, newProgram2 );

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Crossover children have " << newProgram1.getProgramLength() << " and " << newProgram2.getProgramLength() << " lines." << Qt::endl;
#endif

	QDateTime actTime = QDateTime::currentDateTime();

	newProgram1.clampLengthToLimits( minLength,
	                                 maxLength,
	                                 randomizer,
	                                 languageParameters,
	                                 gpParameter.getInstructionProbabilities(),
	                                 historyInfo );

	if( historyInfo > 0 )
		crossoverInd1->addLengthIncreasedInfo( actTime, historyInfo );
	if( historyInfo < 0 )
		crossoverInd1->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

	newProgram2.clampLengthToLimits( minLength,
	                                 maxLength,
	                                 randomizer,
	                                 languageParameters,
	                                 gpParameter.getInstructionProbabilities(),
	                                 historyInfo );

	if( historyInfo > 0 )
		crossoverInd2->addLengthIncreasedInfo( actTime, historyInfo );
	if( historyInfo < 0 )
		crossoverInd2->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

	crossoverInd1->addCrossOverInfo( winner1.getName(),
	                                 winner2.getName(),
	                                 actTime,
	                                 crossPoint1,
	                                 crossPoint2,
	                                 fitness1,
	                                 fitness2 );

	crossoverInd2->addCrossOverInfo( winner1.getName(),
	                                 winner2.getName(),
	                                 actTime,
	                                 crossPoint1,
	                                 crossPoint2,
	                                 fitness1,
	                                 fitness2 );

	QList< SIG_GPIndividual * > crossedInds( 2 );
	crossedInds[ 0 ] = crossoverInd1;
	crossedInds[ 1 ] = crossoverInd2;

	return crossedInds;
}

void SIGEL_GP::SIG_GPOperations::onePointCrossOver( SIGEL_Program::SIG_Program const &winnerProgram1,
                                                    SIGEL_Program::SIG_Program const &winnerProgram2,
                                                    long int crossPoint1,
                                                    long int crossPoint2,
                                                    SIGEL_Program::SIG_Program &newProgram1,
                                                    SIGEL_Program::SIG_Program &newProgram2 )
{
	// copy first part of program 1
	for (long int i = 0; i < crossPoint1; i++)
	{
		newProgram1.appendLine( winnerProgram1.getLine( i ) );
	}

	// append second part of program 2
	for (long int j = crossPoint2; j < winnerProgram2.getProgramLength(); j++)
	{
		newProgram1.appendLine( winnerProgram2.getLine( j ) );
	}

	// copy first part of program 2
	for (long int k = 0; k < crossPoint2; k++)
	{
		newProgram2.appendLine( winnerProgram2.getLine( k ) );
	}

	// append second part of program 1
	for (long int l = crossPoint1; l < winnerProgram1.getProgramLength(); l++)
	{
		newProgram2.appendLine( winnerProgram1.getLine( l ) );
	}
}

void SIGEL_GP::SIG_GPOperations::twoPointCrossOver( SIGEL_Program::SIG_Program const &winnerProgram1,
                                                    SIGEL_Program::SIG_Program const &winnerProgram2,
                                                    long int crossPoint1,
                                                    long int crossPoint2,
                                                    long int segLen1,
                                                    long int segLen2,
                                                    SIGEL_Program::SIG_Program &newProgram1,
                                                    SIGEL_Program::SIG_Program &newProgram2 )
{
	// newprog1 -- copy first part of program 1
	for (long int i = 0; i < crossPoint1; i++)
	{
		newProgram1.appendLine( winnerProgram1.getLine( i ) );
	}

	// newprog1 -- append sequence of program 2
	for (long int j = crossPoint2; j <= crossPoint2+segLen2; j++)
	{
		newProgram1.appendLine( winnerProgram2.getLine( j ) );
	}

	// newprog1 -- eventually append rest of program 1
	for (long int k = crossPoint1+segLen1+1; k < winnerProgram1.getProgramLength(); k++)
	{
		newProgram1.appendLine( winnerProgram1.getLine( k ) );
	}

	// newprog2 -- program 2, part A
	for (long int l = 0; l < crossPoint2; l++)
	{
		newProgram2.appendLine( winnerProgram2.getLine( l ) );
	}

	// newprog2 -- program 1 c/o-seq.
	for (long int m = crossPoint1; m <= crossPoint1+segLen1; m++)
	{
		newProgram2.appendLine( winnerProgram1.getLine( m ) );
	}

	// newprog2 -- program 2, part B
	for (long int n = crossPoint2+segLen2+1; n < winnerProgram2.getProgramLength(); n++)
	{
		newProgram2.appendLine( winnerProgram2.getLine( n ) );
	}
}

/*------------------------------------------Mutation-----------------------------------------*/
SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPOperations::mutation( SIGEL_GP::SIG_GPIndividual& winner,
                                                                  int poolPos,
                                                                  QString newName,
                                                                  SIGEL_Tools::SIG_Randomizer& randomizer,
                                                                  SIGEL_GP::SIG_GPParameter& gpParameter,
                                                                  SIGEL_Robot::SIG_LanguageParameters &languageParameters)
{
	double const winnerFitness      = winner.getFitness();
	MutationType mutationType       = static_cast<MutationType>( randomizer.getRandomInt( mutationTypeCount ) );
	long const minLength            = gpParameter.getMinIndLength();
	long const maxLength            = gpParameter.getMaxIndLength();

	SIG_GPIndividual *mutatedInd = new SIG_GPIndividual( SIG_GPParameter(),
	                                                     winner.getHistory(),
	                                                     newName,
	                                                     QDateTime(),
	                                                     poolPos );

	SIGEL_Program::SIG_Program &newProgram = mutatedInd->getProgramVar();
	newProgram                             = winner.getProgramVar();

	int historyInfo;

	// Check length of (winner) program if length is greater (or equal) than minLength and
	// smaller (or equal) than maxLength:
	newProgram.clampLengthToLimits( minLength,
	                                maxLength,
	                                randomizer,
	                                languageParameters,
	                                gpParameter.getInstructionProbabilities(),
	                                historyInfo );

	// Compute randomly mutation point:
	long int const mutPoint = randomizer.getRandomLong( newProgram.getProgramLength() );

	// -----------------------------------------
	// An existing program line will be mutated:
	// -----------------------------------------
	if( mutationType == MutationType::ChangeLine )
	{
		LineMutationType const lineMutationType = static_cast<LineMutationType>( randomizer.getRandomInt( lineMutationTypeCount ) );

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Mutation changes line " << mutPoint << ", kind " << static_cast<int>( lineMutationType ) << " (0: new line, 1: operand, 2: instruction)." << Qt::endl;
#endif

		switch( lineMutationType )
		{
		case LineMutationType::ReplaceLine:
			replaceLineMutation( newProgram, mutPoint, randomizer, languageParameters, gpParameter.getInstructionProbabilities() );
			break;

		case LineMutationType::ChangeOperand:
			changeOperandMutation( newProgram, mutPoint, randomizer );
			break;

		case LineMutationType::ChangeInstruction:
			changeInstructionMutation( newProgram, mutPoint, randomizer, languageParameters, gpParameter.getInstructionProbabilities() );
			break;
		}

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Mutation result: ";
		newProgram.getLine( mutPoint ).print();
#endif
	}

	if( mutationType == MutationType::InsertLine )
	{
		if( !insertLineMutation( newProgram, mutPoint, randomizer, gpParameter, languageParameters ) )
			mutationType = MutationType::DeleteLine;
	}

	if( mutationType == MutationType::DeleteLine )
	{
		if( newProgram.getProgramLength() - 1 >= minLength )
		{
			deleteLineMutation( newProgram, mutPoint );
		}
		else
		{
			insertLineMutation( newProgram, mutPoint, randomizer, gpParameter, languageParameters );
		}
	}

	QDateTime const actTime = QDateTime::currentDateTime();

	if( historyInfo > 0 )
		mutatedInd->addLengthIncreasedInfo( actTime, historyInfo );
	if( historyInfo < 0 )
		mutatedInd->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

	mutatedInd->addMutationInfo( winner.getName(), actTime, mutPoint, winnerFitness );

	return *mutatedInd;
}

void SIGEL_GP::SIG_GPOperations::replaceLineMutation( SIGEL_Program::SIG_Program &newProgram,
                                                      long int mutPoint,
                                                      SIGEL_Tools::SIG_Randomizer &randomizer,
                                                      SIGEL_Robot::SIG_LanguageParameters &languageParameters,
                                                      QList< int > &instructionProbabilities )
{
	SIGEL_Program::SIG_ProgramLine mutatedLine = newProgram.getLine( mutPoint );
	mutatedLine.randomRobotInstruction( languageParameters,
	                                    randomizer,
	                                    instructionProbabilities );
	newProgram.setLine( mutPoint, mutatedLine );
}

void SIGEL_GP::SIG_GPOperations::changeOperandMutation( SIGEL_Program::SIG_Program &newProgram,
                                                        long int mutPoint,
                                                        SIGEL_Tools::SIG_Randomizer &randomizer )
{
	SIGEL_Program::RobotInstruction const instructionType = newProgram.getLine( mutPoint ).getRobotInstructionType();
	long int const numberOfOperands = newProgram.getLine( mutPoint ).getNumberOfElements();
	int op1 = 0;
	int op2 = 0;

	if( numberOfOperands == 2 )
	{
		// If the number of operands is two, select one randomly, change it randomly,
		// and let the other operand untouched:
		if( randomizer.getRandomInt( 2 ) == 0 )
		{
			op1 = randomOperand( randomizer );
			op2 = newProgram.getLine( mutPoint ).getInstructionElement( 1 );
		}
		else
		{
			op1 = newProgram.getLine( mutPoint ).getInstructionElement( 0 );
			op2 = randomOperand( randomizer );
		}
	}
	else if( instructionType != SIGEL_Program::NOP )
	{
		// The current program line has only one operand that can be changed:
		op1 = randomOperand( randomizer );
	}

	// Set the new program line properties (in this case the operands):
	SIGEL_Program::SIG_ProgramLine mutatedLine = newProgram.getLine( mutPoint );
	mutatedLine.setRobotInstruction( instructionType,
	                                 op1,
	                                 op2 );
	newProgram.setLine( mutPoint, mutatedLine );
}

void SIGEL_GP::SIG_GPOperations::changeInstructionMutation( SIGEL_Program::SIG_Program &newProgram,
                                                            long int mutPoint,
                                                            SIGEL_Tools::SIG_Randomizer &randomizer,
                                                            SIGEL_Robot::SIG_LanguageParameters &languageParameters,
                                                            QList< int > &instructionProbabilities )
{
	auto const [oldOperand1, oldOperand2] = readOperands( newProgram.getLine( mutPoint ) );

	// Create a completely new program line to take the mutated data from:
	SIGEL_Program::SIG_ProgramLine mutatedLine( randomizer,
	                                            languageParameters,
	                                            instructionProbabilities );

	// The new instruction keeps the operands the old line has:
	if( oldOperand1.has_value() && mutatedLine.getNumberOfElements() >= 1 )
		mutatedLine.setElement( 0, oldOperand1.value() );

	if( oldOperand2.has_value() && mutatedLine.getNumberOfElements() == 2 )
		mutatedLine.setElement( 1, oldOperand2.value() );

	newProgram.setLine( mutPoint, mutatedLine );
}

std::pair<std::optional<int>, std::optional<int>> SIGEL_GP::SIG_GPOperations::readOperands( SIGEL_Program::SIG_ProgramLine const &line )
{
	if( line.getRobotInstructionType() == SIGEL_Program::NOP )
		return { std::nullopt, std::nullopt };

	if( line.getNumberOfElements() == 2 )
		return { line.getInstructionElement( 0 ), line.getInstructionElement( 1 ) };

	return { line.getInstructionElement( 0 ), std::nullopt };
}

int SIGEL_GP::SIG_GPOperations::randomOperand( SIGEL_Tools::SIG_Randomizer &randomizer )
{
	int const operand = randomizer.getRandomInt( SIGEL_Tools::SIG_Randomizer::fullRandomIntRange );
	if( randomizer.getRandomInt( 2 ) == 1 )
		return -operand;

	return operand;
}

bool SIGEL_GP::SIG_GPOperations::insertLineMutation( SIGEL_Program::SIG_Program &newProgram,
                                                     long int mutPoint,
                                                     SIGEL_Tools::SIG_Randomizer &randomizer,
                                                     SIGEL_GP::SIG_GPParameter &gpParameter,
                                                     SIGEL_Robot::SIG_LanguageParameters &languageParameters )
{
	if( gpParameter.isMaxIndLengthLimited() &&
	    newProgram.getProgramLength() + 1 > gpParameter.getMaxIndLength() )
		return false;

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Mutation inserts a line at " << mutPoint << "." << Qt::endl;
#endif

	SIGEL_Program::SIG_ProgramLine newProgLine( randomizer, languageParameters, gpParameter.getInstructionProbabilities() );

	newProgram.insertLine( mutPoint, newProgLine );
	return true;
}

void SIGEL_GP::SIG_GPOperations::deleteLineMutation( SIGEL_Program::SIG_Program &newProgram,
                                                     long int mutPoint )
{
#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Mutation deletes line " << mutPoint << "." << Qt::endl;
#endif

	newProgram.deleteLine( mutPoint );
}

/*------------------------------------------Reproduction-----------------------------------------*/
SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPOperations::reproduction( SIGEL_GP::SIG_GPIndividual& winner,
                                                                      QString newName,
                                                                      int poolPos,
                                                                      SIGEL_Tools::SIG_Randomizer& randomizer,
                                                                      SIGEL_GP::SIG_GPParameter& gpParameter,
                                                                      SIGEL_Robot::SIG_LanguageParameters &languageParameters )
{
	SIGEL_GP::SIG_GPIndividual *reproducedInd = new SIGEL_GP::SIG_GPIndividual( SIG_GPParameter(),
	                                                                            winner.getHistory(),
	                                                                            newName,
	                                                                            QDateTime(),
	                                                                            poolPos );

	reproducedInd->setFitness( winner.getFitness() );

	long const minLength = gpParameter.getMinIndLength();
	long const maxLength = gpParameter.getMaxIndLength();
	int  historyInfo;

	reproducedInd->getProgramVar() = winner.getProgramVar();

	reproducedInd->getProgramVar().clampLengthToLimits( minLength,
	                                                    maxLength,
	                                                    randomizer,
	                                                    languageParameters,
	                                                    gpParameter.getInstructionProbabilities(),
	                                                    historyInfo );

	QDateTime actTime = QDateTime::currentDateTime();

	if( historyInfo > 0 )
		reproducedInd->addLengthIncreasedInfo( actTime, historyInfo );
	if( historyInfo < 0 )
		reproducedInd->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

	reproducedInd->addReproductionInfo( winner.getName(), actTime );

	// The reproduced individual is returned
	return *reproducedInd;
}
