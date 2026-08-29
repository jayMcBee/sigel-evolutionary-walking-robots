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
#include "SIGEL_GP/SIG_GPCrossOverTournament.h"
#include "SIGEL_Tools/SIG_IO.h"

#include "MT_Control/MT_Classifier.h"

SIGEL_GP::SIG_GPCrossOverTournament::SIG_GPCrossOverTournament(SIGEL_Tools::SIG_Randomizer& randomizer,
							       SIGEL_GP::SIG_GPFitnessTrainer& fitTrain,
							       SIGEL_GP::SIG_GPPopulation& actPool,
							       SIGEL_GP::SIG_GPParameter& gpParameter,
							       SIGEL_Robot::SIG_LanguageParameters &languageP,
							       int indPos1,
							       int indPos2,
							       int indPos3,
							       int indPos4)
  : SIG_GPTournament(randomizer,fitTrain,actPool,gpParameter,languageP),
    name1( actPool.getNextIdentifier() ),
    name2( actPool.getNextIdentifier() )
{
  indis.resize( 4 );
  indis.insert( 0, new SIG_GPTournamentIndividual( indPos1 ) );
  indis.insert( 1, new SIG_GPTournamentIndividual( indPos2 ) );
  indis.insert( 2, new SIG_GPTournamentIndividual( indPos3 ) );
  indis.insert( 3, new SIG_GPTournamentIndividual( indPos4 ) );
};

SIGEL_GP::SIG_GPCrossOverTournament::~SIG_GPCrossOverTournament()
{ };

bool SIGEL_GP::SIG_GPCrossOverTournament::run()
{
  SIG_GPIndividual &ind1_1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind1_2 = gpPool.getIndividual( indis[1]->indNumber );
  SIG_GPIndividual &ind2_1 = gpPool.getIndividual( indis[2]->indNumber );
  SIG_GPIndividual &ind2_2 = gpPool.getIndividual( indis[3]->indNumber );

   //Helper positions of the individuals
   
   int popos1_1=ind1_1.getPoolPos();
   int popos1_2=ind1_2.getPoolPos();
   int popos2_1=ind2_1.getPoolPos();
   int popos2_2=ind2_2.getPoolPos();

   //Fitness values of the individuals

   double fitness1_1=ind1_1.getFitness();
   double fitness1_2=ind1_2.getFitness();
   double fitness2_1=ind2_1.getFitness();
   double fitness2_2=ind2_2.getFitness();
 
   
// META META META META META META META 
	if( fitness1_1 < -2.0 )
		fitness1_1 = (fitness1_1 + 2.0) * -1.0;
	 
	if( fitness1_2 < -2.0 )
		fitness1_2 = (fitness1_2 + 2.0) * -1.0;
	
	if( fitness2_1 < -2.0 )
		fitness2_1 = (fitness2_1 + 2.0) * -1.0;

	if( fitness2_2 < -2.0 )
		fitness2_2 = (fitness2_2 + 2.0) * -1.0;

// META META META META META META META 
   

   
   SIG_GPIndividual *winner1 = 0;
   SIG_GPIndividual *winner2 = 0;
   int looserPos1 = 0;
   int looserPos2 = 0;
   int winnerPos1 = 0;
   int winnerPos2 = 0;

   //The tournament action
   

   if (fitness1_1 >= fitness1_2)
     {
       winner1 = &ind1_1;
       winnerPos1 = popos1_1;
       looserPos1 = popos1_2;
     }
   else
     {
       winner1 = &ind1_2;
       winnerPos1 = popos1_2;
       looserPos1 = popos1_1;
     };

   if (fitness2_1 >= fitness2_2)
     {
       winner2 = &ind2_1;
       winnerPos2 = popos2_1;
       looserPos2 = popos2_2;
     }
   else
     {
       winner2 = &ind2_2;
       winnerPos2 = popos2_2;
       looserPos2 = popos2_1;
     };

#ifdef SIG_DEBUG
   SIGEL_Tools::SIG_IO::cerr <<"\nthe Individuals " <<winnerPos1<<" and "<<winnerPos2<<" winns the Tournament and will be crossed..\n";
#endif

   QList< SIG_GPIndividual * > cinds = SIG_GPOperations::crossOver(*winner1,
								   looserPos1,
								   *winner2,
								   looserPos2,
								   name1,
								   name2,
								   randomizer,
								   gpParameter,
								   languageP);

#ifdef SIG_DEBUG
   SIGEL_Tools::SIG_IO::cerr <<"the Individuals have been crossed and inserted at the Pool Positions " <<looserPos1<<" and "<<looserPos2<<" .\n";
#endif

   SIG_GPIndividual &looser1 = gpPool.getIndividual( looserPos1 );
   SIG_GPIndividual &looser2 = gpPool.getIndividual( looserPos2 );

   inhume( looser1 );
   inhume( looser2 );

   gpPool.setIndividual(*cinds[0],looserPos1);
   gpPool.setIndividual(*cinds[1],looserPos2);
   
   return true;
};


bool SIGEL_GP::SIG_GPCrossOverTournament::run(MT_Classifier *MetaClassifier)
{

  SIG_GPIndividual &ind1_1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind1_2 = gpPool.getIndividual( indis[1]->indNumber );
  SIG_GPIndividual &ind2_1 = gpPool.getIndividual( indis[2]->indNumber );
  SIG_GPIndividual &ind2_2 = gpPool.getIndividual( indis[3]->indNumber );

   //Helper positions of the individuals
   
   int popos1_1=ind1_1.getPoolPos();
   int popos1_2=ind1_2.getPoolPos();
   int popos2_1=ind2_1.getPoolPos();
   int popos2_2=ind2_2.getPoolPos();

   //Fitness values of the individuals

   double fitness1_1=ind1_1.getFitness();
   double fitness1_2=ind1_2.getFitness();
   double fitness2_1=ind2_1.getFitness();
   double fitness2_2=ind2_2.getFitness();
   
   	// Creates two training cases for the meta GP system
   MetaClassifier->createNewTCase(&const_cast<SIGEL_Program::SIG_Program&>(ind1_1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind1_2.getProgram()),fitness1_1-fitness1_2);
   MetaClassifier->createNewTCase(&const_cast<SIGEL_Program::SIG_Program&>(ind2_1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind2_2.getProgram()),fitness2_1-fitness2_2);
   
   SIG_GPIndividual *winner1 = 0;
   SIG_GPIndividual *winner2 = 0;
   int looserPos1 = 0;
   int looserPos2 = 0;
   int winnerPos1 = 0;
   int winnerPos2 = 0;

   //The tournament action
   
   if (fitness1_1 >= fitness1_2)
     {
       winner1 = &ind1_1;
       winnerPos1 = popos1_1;
       looserPos1 = popos1_2;
     }
   else
     {
       winner1 = &ind1_2;
       winnerPos1 = popos1_2;
       looserPos1 = popos1_1;
     };

   if (fitness2_1 >= fitness2_2)
     {
       winner2 = &ind2_1;
       winnerPos2 = popos2_1;
       looserPos2 = popos2_2;
     }
   else
     {
       winner2 = &ind2_2;
       winnerPos2 = popos2_2;
       looserPos2 = popos2_1;
     };

   QList< SIG_GPIndividual * > cinds = SIG_GPOperations::crossOver(*winner1,
								   looserPos1,
								   *winner2,
								   looserPos2,
								   name1,
								   name2,
								   randomizer,
								   gpParameter,
								   languageP);

   SIG_GPIndividual &looser1 = gpPool.getIndividual( looserPos1 );
   SIG_GPIndividual &looser2 = gpPool.getIndividual( looserPos2 );

   inhume( looser1 );
   inhume( looser2 );

   gpPool.setIndividual(*cinds[0],looserPos1);
   gpPool.setIndividual(*cinds[1],looserPos2);
   
   return true;

};

bool  SIGEL_GP::SIG_GPCrossOverTournament::classify(MT_Classifier *MetaClassifier)
{
 
  SIG_GPIndividual &ind1_1 = gpPool.getIndividual( indis[0]->indNumber );
  SIG_GPIndividual &ind1_2 = gpPool.getIndividual( indis[1]->indNumber );
  SIG_GPIndividual &ind2_1 = gpPool.getIndividual( indis[2]->indNumber );
  SIG_GPIndividual &ind2_2 = gpPool.getIndividual( indis[3]->indNumber );

   //Helper positions of the individuals
   
   int popos1_1=ind1_1.getPoolPos();
   int popos1_2=ind1_2.getPoolPos();
   int popos2_1=ind2_1.getPoolPos();
   int popos2_2=ind2_2.getPoolPos();

   SIG_GPIndividual *winner1 = 0;
   SIG_GPIndividual *winner2 = 0;
   int looserPos1 = 0;
   int looserPos2 = 0;
   int winnerPos1 = 0;
   int winnerPos2 = 0;

   //The tournament action
   
  // Tournament winner determined via the classifier;
   // FitDiff < 0 -> indi2 wins;  FitDiff >= 0 -> indi1 wins
	double  FitDiff1 = MetaClassifier->classifier(&const_cast<SIGEL_Program::SIG_Program&>(ind1_1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind1_2.getProgram()));	
  	double  FitDiff2 = MetaClassifier->classifier(&const_cast<SIGEL_Program::SIG_Program&>(ind2_1.getProgram()), &const_cast<SIGEL_Program::SIG_Program&>(ind2_2.getProgram()));	
  
   if (FitDiff1 >= 0)
     {
       winner1 = &ind1_1;
       winnerPos1 = popos1_1;
       looserPos1 = popos1_2;
     }
   else
     {
       winner1 = &ind1_2;
       winnerPos1 = popos1_2;
       looserPos1 = popos1_1;
     };

   if (FitDiff2 >= 0)
     {
       winner2 = &ind2_1;
       winnerPos2 = popos2_1;
       looserPos2 = popos2_2;
     }
   else
     {
       winner2 = &ind2_2;
       winnerPos2 = popos2_2;
       looserPos2 = popos2_1;
     };

   QList< SIG_GPIndividual * > cinds = SIG_GPOperations::crossOver(*winner1,
								   looserPos1,
								   *winner2,
								   looserPos2,
								   name1,
								   name2,
								   randomizer,
								   gpParameter,
								   languageP);

   SIG_GPIndividual &looser1 = gpPool.getIndividual( looserPos1 );
   SIG_GPIndividual &looser2 = gpPool.getIndividual( looserPos2 );

   inhume( looser1 );
   inhume( looser2 );

   gpPool.setIndividual(*cinds[0],looserPos1);
   gpPool.setIndividual(*cinds[1],looserPos2);
   
   return true;

};