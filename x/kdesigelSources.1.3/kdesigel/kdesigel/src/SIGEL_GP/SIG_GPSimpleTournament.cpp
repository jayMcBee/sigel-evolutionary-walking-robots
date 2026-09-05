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
#include "SIGEL_GP/SIG_GPSimpleTournament.h"
#include "SIGEL_Tools/SIG_IO.h"

#include "MT_Control/MT_Classifier.h"

SIGEL_GP::SIG_GPSimpleTournament::SIG_GPSimpleTournament(SIGEL_Tools::SIG_Randomizer& randomizer,
							 SIG_GPFitnessTrainer& trainer,
							 SIG_GPPopulation& actPool,
							 SIG_GPParameter& gpParameter,
							 SIGEL_Robot::SIG_LanguageParameters &languageP,
							 int ppos1,
							 int ppos2)
  : SIG_GPTournament(randomizer,trainer,actPool,gpParameter,languageP),
    name(actPool.getNextIdentifier())
{
  indis.resize( 2 );
  indis[ 0 ] = new SIG_GPTournamentIndividual( ppos1 );
  indis[ 1 ] = new SIG_GPTournamentIndividual( ppos2 );
};

SIGEL_GP::SIG_GPSimpleTournament::~SIG_GPSimpleTournament()
{ };

bool SIGEL_GP::SIG_GPSimpleTournament::run()
{      
   //Positions of the two participants

  SIG_GPIndividual &ind1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind2 = gpPool.getIndividual( indis[1]->indNumber );

  int popos1=ind1.getPoolPos();
  int popos2=ind2.getPoolPos();
 
   //Fitness values of the two participants

   double var1=ind1.getFitness();
   double var2=ind2.getFitness();


// META META META META META META META 
// ATTENTION !!! Giga-important code !!!
	if( var1 < -2.0 )
		var1 = (var1 + 2.0) * -1.0;
	
	if( var2 < -2.0 )
		var2 = (var2 + 2.0) * -1.0;
	
// META META META META META META META 
   
   //The tournament action
   
   SIG_GPIndividual *winner = 0;
   int looserPos = 0;
   int winnerPos = 0;

   if(var1>=var2)
     {
       winner = &ind1;
       looserPos = popos2;
       winnerPos = popos1;
     }
   else
     {
       winner = &ind2;
       looserPos = popos1;
       winnerPos = popos2;
     };

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr <<"\nthe Individual " <<winnerPos<<" winns the Tournament and will be reproduced..\n";
#endif

     SIG_GPIndividual& rind = SIG_GPOperations::reproduction( *winner,
							       name,
							       looserPos,
							       randomizer,
							       gpParameter,
							       languageP );

     rind.setFitness( winner->getFitness() );

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr <<"the Individual " <<winnerPos<<" have been reproduced and inserted at the Pool Position " << looserPos << " .\n";
#endif

    SIG_GPIndividual &looser = gpPool.getIndividual( looserPos );

    inhume( looser );

    gpPool.setIndividual(rind, looserPos);

    return true; 
 };
 

bool SIGEL_GP::SIG_GPSimpleTournament::run(MT_Classifier *MetaClassifier)
{

  //Positions of the two participants

  SIG_GPIndividual &ind1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind2 = gpPool.getIndividual( indis[1]->indNumber );

  int popos1=ind1.getPoolPos();
  int popos2=ind2.getPoolPos();
 
   //Fitness values of the two participants

   double var1=ind1.getFitness();
   double var2=ind2.getFitness();
   

	// Creates a training case in the meta GP system
   MetaClassifier->createNewTCase(&const_cast<SIGEL_Program::SIG_Program&>(ind1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind2.getProgram()),var1-var2);


   //The tournament action
   
   SIG_GPIndividual *winner = 0;
   int looserPos = 0;
   int winnerPos = 0;

   if(var1>=var2)
     {
       winner = &ind1;
       looserPos = popos2;
       winnerPos = popos1;
     }
   else
     {
       winner = &ind2;
       looserPos = popos1;
       winnerPos = popos2;
     };

     SIG_GPIndividual& rind = SIG_GPOperations::reproduction( *winner,
							       name,
							       looserPos,
							       randomizer,
							       gpParameter,
							       languageP );

     rind.setFitness( winner->getFitness() );

    SIG_GPIndividual &looser = gpPool.getIndividual( looserPos );

    inhume( looser );

    gpPool.setIndividual(rind, looserPos);

    return true; 
};

bool SIGEL_GP::SIG_GPSimpleTournament::classify(MT_Classifier *MetaClassifier)
{
   //Positions of the two participants

  SIG_GPIndividual &ind1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind2 = gpPool.getIndividual( indis[1]->indNumber );

  int popos1=ind1.getPoolPos();
  int popos2=ind2.getPoolPos();
 

    // Determine the tournament winner via the classifier;
   // FitDiff < 0 -> indi2 wins;  FitDiff >= 0 -> indi1 wins
	double  FitDiff = MetaClassifier->classifier(&const_cast<SIGEL_Program::SIG_Program&>(ind1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind2.getProgram()));	

  //The tournament action
   
   SIG_GPIndividual *winner = 0;
   int looserPos = 0;
   int winnerPos = 0;

   if(FitDiff>=0)
     {
       winner = &ind1;
       looserPos = popos2;
       winnerPos = popos1;
     }
   else
     {
       winner = &ind2;
       looserPos = popos1;
       winnerPos = popos2;
     };

     SIG_GPIndividual& rind = SIG_GPOperations::reproduction( *winner,
							       name,
							       looserPos,
							       randomizer,
							       gpParameter,
							       languageP );

     rind.setFitness( winner->getFitness() );

    SIG_GPIndividual &looser = gpPool.getIndividual( looserPos );

    inhume( looser );

    gpPool.setIndividual(rind, looserPos);

    return true;


};
