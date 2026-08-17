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
#include "SIGEL_GP/SIG_GPOperations.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include <qarray.h>
#include <qstringlist.h>

/*------------------------------------------Crossover-----------------------------------------*/

QVector<SIGEL_GP::SIG_GPIndividual> SIGEL_GP::SIG_GPOperations::crossOver( SIGEL_GP::SIG_GPIndividual& winner1,
									   int poolPos1,
									   SIG_GPIndividual& winner2,
									   int poolPos2,
									   QString newName1,
									   QString newName2,
									   SIGEL_Tools::SIG_Randomizer& randomizer,
									   SIGEL_GP::SIG_GPParameter& gpParameter,
									   SIGEL_Robot::SIG_LanguageParameters &languageP)
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

  // prepare length of segments to be swapped if 2-point-variant is chosen;
  // max. 1/10 of program length, at least 1 instruction
  long int segLen1 = randomizer.getRandomLong( (winnerProgram1.getProgramLength()/10) ) + 1;
  if ((crossPoint1+segLen1) >= winnerProgram1.getProgramLength())
  {  segLen1 = winnerProgram1.getProgramLength() - crossPoint1 - 1;
  }

  long int segLen2 = randomizer.getRandomLong( (winnerProgram2.getProgramLength()/10) ) + 1;
  if ((crossPoint2+segLen2) >= winnerProgram2.getProgramLength())
  {  segLen2 = winnerProgram2.getProgramLength() - crossPoint2 - 1;
  }

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "\n<CrossOver> Info: ProgramLength = " << minLength;

	if( maxLength==0 )
  {	SIGEL_Tools::SIG_IO::cerr << " - no limit\n";
	}
	else
	{	SIGEL_Tools::SIG_IO::cerr << " - " << maxLength << "\n";
	}
#endif

  // check new program lengths if min. program length was requested
  if( minLength > 0 )
    {
      if( ( crossPoint1 + winnerProgram2.getProgramLength() - crossPoint2 < minLength ) ||
          ( crossPoint2 + winnerProgram1.getProgramLength() - crossPoint1 < minLength ) )
	{
          crossPoint1 = winnerProgram1.getProgramLength() / 2;
          if ((crossPoint1+segLen1) >= winnerProgram1.getProgramLength())
          {  segLen1 = winnerProgram1.getProgramLength() - crossPoint1 - 1;
          }

          crossPoint2 = winnerProgram2.getProgramLength() / 2;
				  if ((crossPoint2+segLen2) >= winnerProgram2.getProgramLength())
          {  segLen2 = winnerProgram2.getProgramLength() - crossPoint2 - 1;
          }

#ifdef SIG_DEBUG

          SIGEL_Tools::SIG_IO::cerr << "\n-> CrossOver points have been set to "
				    << crossPoint1
				    << " and "
	                            << crossPoint2
	                            << "\nThe resulting legths are: "
                                    << crossPoint1 + winnerProgram2.getProgramLength() - crossPoint2
                                    << " and "
                                    << crossPoint2 + winnerProgram1.getProgramLength() - crossPoint1;
#endif
        }
    }

	// randomly select any recombination type; currently the 2-point-c/o
	// has a chance of 3:2 to be selected !
  long const generalRecombinationType = randomizer.getRandomInt(5);

	SIGEL_Program::SIG_ProgramLine *newProgLine = 0;
  SIGEL_Program::SIG_ProgramLine *sourceProgLine = 0;

	// Recombination/Crossover, Variant 1:
	// (just guessing from the code, obviosly nobody thought it was worth commenting..  -jan)
	// Simple 1 point crossover, create two new programs
	if( generalRecombinationType == 0 )
	{

		// copy first part of program 1
		for (long int i = 0; i < crossPoint1; i++)
		{
	  	newProgLine = new SIGEL_Program::SIG_ProgramLine();
	  	sourceProgLine = winnerProgram1.getLine( i );
	  	*newProgLine = *sourceProgLine;
	  	newProgram1.appendLine( newProgLine );
		}

		// append second part of program 2
		for (long int j = crossPoint2; j < winnerProgram2.getProgramLength(); j++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( j );
			*newProgLine = *sourceProgLine;
			newProgram1.appendLine( newProgLine );
		}

		// the other way round:  copy first part of prog. 2
		for (long int k = 0; k < crossPoint2; k++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( k );
			*newProgLine = *sourceProgLine;
			newProgram2.appendLine( newProgLine );
		}

		// you might have guessed it: append 2. part of first program
		for (long int l = crossPoint1; l < winnerProgram1.getProgramLength(); l++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram1.getLine( l );
			*newProgLine = *sourceProgLine;
			newProgram2.appendLine( newProgLine );
		}
	}

	// Recombination/Crossover, Variant 2:
	// (Do *you* detect any meaningful difference ..?  The implementor seemed to be a bit confused..)
	// Simple 1 point crossover, again creating two new programs
	if( generalRecombinationType == 1 )
	{
		for( long int i = 0; i < crossPoint2; i++ )
		{
			newProgLine    = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( i );
			*newProgLine   = *sourceProgLine;

			newProgram2.appendLine( newProgLine );
		}

		for( long int j = crossPoint1; j < winnerProgram1.getProgramLength(); j++ )
		{
				newProgLine    = new SIGEL_Program::SIG_ProgramLine();
				sourceProgLine = winnerProgram1.getLine( j );
				*newProgLine   = *sourceProgLine;

				newProgram2.appendLine( newProgLine );
		}

		for( long int k = 0; k < crossPoint1; k++ )
		{
			newProgLine    = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram1.getLine( k );
			*newProgLine   = *sourceProgLine;

			newProgram1.appendLine( newProgLine );
		}

		for( long int m = crossPoint2; m < winnerProgram2.getProgramLength(); m++ )
		{
			newProgLine    = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( m );
			*newProgLine   = *sourceProgLine;

			newProgram1.appendLine( newProgLine );
		}
	}

	// Recombination/Crossover, Variant 3:
	// 2 point crossover preferring short sequences (max. 1/10 of program length), creates two new programs
	if (generalRecombinationType >= 2)
	{
		// newprog1 -- copy first part of program 1
		for (long int i = 0; i < crossPoint1; i++)
		{
	  	newProgLine = new SIGEL_Program::SIG_ProgramLine();
	  	sourceProgLine = winnerProgram1.getLine( i );
	  	*newProgLine = *sourceProgLine;

	  	newProgram1.appendLine( newProgLine );
		}

		// newprog1 -- append sequence of program 2
		for (long int j = crossPoint2; j <= crossPoint2+segLen2; j++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( j );
			*newProgLine = *sourceProgLine;
			newProgram1.appendLine( newProgLine );
		}

		// newprog1 -- eventually append rest of program 1
		for (long int k = crossPoint1+segLen1; k < winnerProgram1.getProgramLength(); k++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram1.getLine( k );
			*newProgLine = *sourceProgLine;
			newProgram1.appendLine( newProgLine );
		}

		// newprog2 -- program 2, part A
		for (long int l = 0; l < crossPoint2; l++)
		{
	  	newProgLine = new SIGEL_Program::SIG_ProgramLine();
	  	sourceProgLine = winnerProgram2.getLine( l );
	  	*newProgLine = *sourceProgLine;
	  	newProgram2.appendLine( newProgLine );
		}

		// newprog2 -- program 1 c/o-seq.
		for (long int m = crossPoint1; m <= crossPoint1+segLen1; m++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram1.getLine( m );
			*newProgLine = *sourceProgLine;
			newProgram2.appendLine( newProgLine );
		}

		// newprog2 -- program 2, part B
		for (long int n = crossPoint2+segLen2; n < winnerProgram2.getProgramLength(); n++)
		{
			newProgLine = new SIGEL_Program::SIG_ProgramLine();
			sourceProgLine = winnerProgram2.getLine( n );
			*newProgLine = *sourceProgLine;
			newProgram2.appendLine( newProgLine );
		}
	}

	// truncate programs if they grew too long thus violating the max. prog. length limit
	long int prgLength1 = newProgram1.getProgramLength();
	long int prgLength2 = newProgram2.getProgramLength();

	if ((maxLength > 0) && (prgLength1 > maxLength))
	{

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "\n-> new program (1) too long (length=" << newProgram1.getProgramLength() << ") -> length decreased\n";
#endif

		for( long int i = maxLength - 1; i < prgLength1; i++ )
		{	newProgram1.deleteLine( i );
		}

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "\n-> new length = " << newProgram1.getProgramLength();
#endif
	}

	if ((maxLength > 0) && (prgLength2 > maxLength))
	{

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "\n-> new program (2) too long (length=" << newProgram2.getProgramLength() << ") -> length decreased\n";
#endif

		for( long int i = maxLength - 1; i < prgLength2; i++ )
    {	newProgram2.deleteLine( i );
		}

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "\n-> new length = " << newProgram2.getProgramLength();
#endif
	}

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "\n-> new program (1) length = " << newProgram1.getProgramLength();
	SIGEL_Tools::SIG_IO::cerr << "\n-> new program (2) length = " << newProgram2.getProgramLength();
#endif

  QDateTime actTime = QDateTime::currentDateTime();

  newProgram1.checkLength( minLength,
			   maxLength,
			   randomizer,
			   languageP,
			   gpParameter.getInstructionProbabilities(),
			   historyInfo );

  if( historyInfo > 0 ) crossoverInd1->addLengthIncreasedInfo( actTime, historyInfo );
  if( historyInfo < 0 ) crossoverInd1->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

  newProgram2.checkLength( minLength,
			   maxLength,
			   randomizer,
			   languageP,
			   gpParameter.getInstructionProbabilities(),
			   historyInfo );

  if( historyInfo > 0 ) crossoverInd2->addLengthIncreasedInfo( actTime, historyInfo );
  if( historyInfo < 0 ) crossoverInd2->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

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

  QVector< SIG_GPIndividual > crossedInds( 2 );
  crossedInds.insert( 0, crossoverInd1 );
  crossedInds.insert( 1, crossoverInd2 );

  return crossedInds;
};




/*------------------------------------------Mutation-----------------------------------------*/

SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPOperations::mutation( SIGEL_GP::SIG_GPIndividual& winner,
								  int poolPos,
								  QString newName,
								  SIGEL_Tools::SIG_Randomizer& randomizer,
								  SIGEL_GP::SIG_GPParameter& gpParameter,
								  SIGEL_Robot::SIG_LanguageParameters &languageP)
{
  double     winnerFitness        = winner.getFitness();
  int        generalMutationType  = randomizer.getRandomInt( 3 );  // 0: Variation of existing program line
                                                                   // 1: Add a new random program line
                                                                   // 2: Delete an existing program line
  int        specializedMutation  = 0;
  long const minLength            = gpParameter.getMinIndLength();
  long const maxLength            = gpParameter.getMaxIndLength();
  int        op1                  = 0;
  int        op2                  = 0;
  SIGEL_Program::Robotinstruction instructionType;
  SIGEL_Program::SIG_ProgramLine  *newProgLine ;
  long int   mutPoint             = 0;
  long int   numberOfOperands     = 0;
  int        historyInfo;

#ifdef SIG_DEBUG

  SIGEL_Tools::SIG_IO::cerr << "\n<MUTATION> ";

#endif            


  SIG_GPIndividual *mutatedInd = new SIG_GPIndividual( SIG_GPParameter(),
						       winner.getHistory(),
						       newName,
						       QDateTime(),
						       poolPos );

  SIGEL_Program::SIG_Program &newProgram = mutatedInd->getProgramVar();
  newProgram                             = winner.getProgramVar();

  // Check length of (winner) program if length is greater (or equal) than minLength and
  // smaller (or equal) than maxLength:

  newProgram.checkLength( minLength,
			  maxLength,
			  randomizer,
			  languageP,
			  gpParameter.getInstructionProbabilities(),
			  historyInfo );

  // Compute randomly mutation point:

  mutPoint          = randomizer.getRandomLong( newProgram.getProgramLength() );

  // Get instruction type of line that is to be mutated:

  instructionType   = newProgram.getLine( mutPoint )->getRobotinstructionType();

  // Get number of operands of line that is to be mutated:

  numberOfOperands  = newProgram.getLine( mutPoint )->getNumberOfElements();

  // Get operands (op1, op2) of line that is to be mutated:

  if( instructionType == SIGEL_Program::NOP )
    { // If number of operands = 0:
     op1 = 0;
     op2 = 0;
    }
  else
    { // Number of operands is >=1:   
     op1 = newProgram.getLine( mutPoint )->getInstructionElement( 0 );

     if( numberOfOperands == 2 )
        // If number of operands = 2: 
        op2 = newProgram.getLine( mutPoint )->getInstructionElement( 1 );

      else
	// If number of operands = 1: 
        op2 = 0;
    }
  
  // -----------------------------------------
  // An existing program line will be mutated:
  // -----------------------------------------

  if( generalMutationType == 0 )
    {

#ifdef SIG_DEBUG

      SIGEL_Tools::SIG_IO::cerr << "\n<TYPE 0>";
      SIGEL_Tools::SIG_IO::cerr << "\nline to mutate: [";
      newProgram.getLine( mutPoint )->print();
      SIGEL_Tools::SIG_IO::cerr << "]"; 

#endif

    specializedMutation  = randomizer.getRandomInt( 3 );  // 0: Replace an existing program line by a new one
                                                          // 1: Change one of the operands
                                                          // 2: Change the instruction 
#ifdef SIG_DEBUG

    SIGEL_Tools::SIG_IO::cerr << "\n<SUBTYPE " << specializedMutation << ">";

#endif


    switch( specializedMutation )
      {
      case 0:
	     // -------------------------------------------------------------------------------
	     // Generate (randomly) a completely new program line by replacing the old content:
	     // -------------------------------------------------------------------------------   

             newProgram.getLine( mutPoint )->randomRobotinstruction( languageP, 
								     randomizer, 
								     gpParameter.getInstructionProbabilities() );
             break;

      case 1:
	     // -------------------------------------------------------------------------------
	     // Mutate a randomly chosen operand. It is important to take care about the
	     // program line type, because the number of operands is dependend on the program
	     // line instruction.
	     // -------------------------------------------------------------------------------   
            
             if( numberOfOperands == 2 )
	        {
		  // If the number of operands is two, select one randomly, change it randomly,
                  // and let the other operand untouched:

                 if( randomizer.getRandomInt( 2 ) == 0 )         
                  {
                   // Operand 1 will be varied:

                   op1 = randomizer.getRandomInt( 32000 );
                   if( randomizer.getRandomInt( 2 ) == 1 ) op1 = (-1) * op1;
                   op2 = newProgram.getLine( mutPoint )->getInstructionElement( 1 );
                  }
                 else                                            
                  {
                   // Operand 2 will be varied:

                   op1 = newProgram.getLine( mutPoint )->getInstructionElement( 0 );
                   op2 = randomizer.getRandomInt( 32000 );
                   if( randomizer.getRandomInt( 2 ) == 1 ) op2 = (-1) * op2;
                  }
	        }
              else
		{
		  if( instructionType != SIGEL_Program::NOP )
		    {
                      // The current program line has only one operand that can be changed:

                      op1 = randomizer.getRandomInt( 32000 );
                      if( randomizer.getRandomInt( 2 ) == 1 ) op1 = (-1) * op1;
		    }
                  else
		    {
                      // The current program line has no operands:

                      op1 = 0;
                      op2 = 0;
		    }
                } 
             
             // Set the new program line properties (in this case the operands):

             newProgram.getLine( mutPoint )->setRobotinstruction( instructionType, 
								  op1, 
								  op2 );
             break;

      case 2:

	     // -------------------------------------------------------------------------------
	     // Change the instruction type. It is very important to take care about the
	     // following case:
	     // If an instruction is consisting of only one onperand, and it shall be mutated
	     // to an instruction that has to contain two operands, the (new) second operand
	     // needs to be initialized with a (randomly) value!
	     // -------------------------------------------------------------------------------
   
	     // Create a completely new program line to take the mutated data from:
    
             newProgLine = new SIGEL_Program::SIG_ProgramLine( randomizer, 
							       languageP, 
							       gpParameter.getInstructionProbabilities() );

             if( newProgLine->getRobotinstructionType() != SIGEL_Program::NOP )
	       {
		 // The number of operands must eventually be increased for the new line, 
                 // because the new instruction type requires more operands. In this case
                 // the operand(s) will be taken from the new generated line (s. above):              

                if( ( newProgLine->getNumberOfElements() >= 1 ) && ( op1 == 0 ) )

                    op1 = newProgLine->getInstructionElement( 0 );
   
                if( ( newProgLine->getNumberOfElements() == 2 ) && ( op2 == 0 ) )
  
                    op2 = newProgLine->getInstructionElement( 1 );
	       }
             else
               {
                 // No further operands are required in case of a NOP instruction:
 
                 op1 = 0;
                 op2 = 0;
	       }   

             newProgram.getLine( mutPoint )->setRobotinstruction( newProgLine->getRobotinstructionType(), 
						 		  op1,
								  op2 );  
             delete newProgLine;

             break;
      }

#ifdef SIG_DEBUG

     SIGEL_Tools::SIG_IO::cerr << "\nresult: [";
     newProgram.getLine( mutPoint )->print();
     SIGEL_Tools::SIG_IO::cerr << "]"; 

#endif

    }

  if( generalMutationType == 1 )
    if( newProgram.getProgramLength() + 1 <= maxLength )
      {

#ifdef SIG_DEBUG

        SIGEL_Tools::SIG_IO::cerr << "\n<TYPE 1>";

#endif

        newProgLine = new SIGEL_Program::SIG_ProgramLine( randomizer, 
							languageP, 
							gpParameter.getInstructionProbabilities() );

        newProgram.insertLine( mutPoint, newProgLine );

      }
    else
      generalMutationType = 2;
   

  if( generalMutationType == 2 )
    if( newProgram.getProgramLength() - 1 >= minLength )
      {

#ifdef SIG_DEBUG

        SIGEL_Tools::SIG_IO::cerr << "\n<TYPE 2>";

#endif

        newProgram.deleteLine( mutPoint );
      }
    else
      {
        if( newProgram.getProgramLength() + 1 <= maxLength )
          {

#ifdef SIG_DEBUG

            SIGEL_Tools::SIG_IO::cerr << "\n<TYPE 1>";

#endif

            newProgLine = new SIGEL_Program::SIG_ProgramLine( randomizer, 
							      languageP, 
							      gpParameter.getInstructionProbabilities() );

            newProgram.insertLine( mutPoint, newProgLine );
          }
      }

#ifdef SIG_DEBUG

             SIGEL_Tools::SIG_IO::cerr << "\n\n";

#endif


  QDateTime actTime = QDateTime::currentDateTime();

  if( historyInfo > 0 ) mutatedInd->addLengthIncreasedInfo( actTime, historyInfo );
  if( historyInfo < 0 ) mutatedInd->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

  mutatedInd->addMutationInfo( winner.getName(), actTime, mutPoint, winnerFitness );

  return  *mutatedInd;
};



/*------------------------------------------Reproduction-----------------------------------------*/

SIGEL_GP::SIG_GPIndividual& SIGEL_GP::SIG_GPOperations::reproduction( SIGEL_GP::SIG_GPIndividual& winner,
								      QString newName,
								      int poolPos,
								      SIGEL_Tools::SIG_Randomizer& randomizer,
								      SIGEL_GP::SIG_GPParameter& gpParameter,
								      SIGEL_Robot::SIG_LanguageParameters &languageP )
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

#ifdef SIG_DEBUG

  SIGEL_Tools::SIG_IO::cerr << "\n<REPRODUCTION>";

  if (!reproducedInd)
    {
      SIGEL_Tools::SIG_IO::cerr << "reproduction: Konnte kein neues Individuum erzeugen.\n";
      exit(1);
    };

#endif

  reproducedInd->getProgramVar() = winner.getProgramVar();

  reproducedInd->getProgramVar().checkLength( minLength,
  					      maxLength,
  					      randomizer,
  					      languageP,
  					      gpParameter.getInstructionProbabilities(),
					      historyInfo );

  QDateTime actTime = QDateTime::currentDateTime();

  if( historyInfo > 0 ) reproducedInd->addLengthIncreasedInfo( actTime, historyInfo );
  if( historyInfo < 0 ) reproducedInd->addLengthDecreasedInfo( actTime, (-1) * historyInfo );

  reproducedInd->addReproductionInfo( winner.getName(), actTime );

  // Das reproduzierte Individuum wird zurueckgegeben

#ifdef SIG_DEBUG

  //SIGEL_Tools::SIG_IO::cerr << "\n<TEST>\n";
  //reproducedInd->print();
  //SIGEL_Tools::SIG_IO::cerr << "\n--> Programm-Laenge:"
  //                          << reproducedInd->getProgramVar().getProgramLength()
  //                          << "\n\n";
#endif  

  return *reproducedInd;
};
