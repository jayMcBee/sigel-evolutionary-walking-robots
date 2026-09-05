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
#ifndef SIGEL_GP_SIG_GPCROSSOVERTOURNAMENT_H
#define SIGEL_GP_SIG_GPCROSSOVERTOURNAMENT_H

#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_GP/SIG_GPTournament.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

namespace SIGEL_GP
{

/**
* This class realizes the special tournament, which ends with a crossover of the winners.
* It inherits its basic structure from SIG_GPTournament and uses the static function
* crossOver of SIG_GPOperations for the genetic operation. 
*/

class SIG_GPCrossOverTournament : public SIGEL_GP::SIG_GPTournament
{ 
	
/**
* The constructor of the crossovertournament.
* @pre
* The tournament creation routine creates a tournament with a crossover of the winners
* and the GPManager wants to compute the tournament. 
* @post
* The crossovertournament object is created.
* @param randomizer
* The randomizer which creates the random mutation point.
* @param fitTrain
* The fitnesstrainer, who controls the fitness computation process.
* @param popula
* The pool, which contains the possible members of the tournament. 
* @param indPool1
* The pool position of tournament member number one.
* @param indPool2
* The pool position of tournament member number two.
* @param indPool3
* The pool position of tournament member number three.
* @param indPool4
* The pool position of tournament member number four.
*/
 public:
  SIG_GPCrossOverTournament(SIGEL_Tools::SIG_Randomizer& randomizer, 
			    SIG_GPFitnessTrainer& fitTrain, 
			    SIG_GPPopulation& actPool, 
			    SIG_GPParameter& gpParameter,
			    SIGEL_Robot::SIG_LanguageParameters &languageP,
			    int indPos1, 
			    int indPos2, 
			    int indPos3, 
			    int indPos4);

     
 private:
  QString name1;
  QString name2;
   


/**
* The destructor of the crossovertournament.
* @pre
* The tournament is played, a crossovered copies of the winners are placed 
* in position of the losers in the pool, the fitnesstask of the new
* individuals are complete and returned the correct fitness.
* @post
* The tournament is destructed.
*/
 public:
virtual ~SIG_GPCrossOverTournament();

/**
* The virtual function effects the tournament to start.
* @pre
* The tournament is created and the GPmanager activates the execution
* of the tournament.
* @post
* The tournament is started.
* @return
* The boolean for successful launching the function.
*/
 public:
virtual bool run();

/*
* see above run(); 
* the tournament participant and the tournament outcome defined
* a new Trainingcase in the Meta GP-System  
* @pre: there is a Meta Classifier Syste
*/
public:
virtual bool run(MT_Classifier *MetaClassifier);

/**
* equal run(), but the tournament outcome will decide by the MT_Classifer;
* exact Fitness of the Sigel Individuals isn't needed ;
* @pre: there is a Meta Classifier System
*   
*/
public:
virtual bool classify(MT_Classifier *MetaClassifier);



};

}
#endif //  SIGEL_GP_SIG_GPCROSSOVER_H
