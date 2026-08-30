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
#ifndef SIGEL_GP_SIG_GPTOURNAMENT_H
#define SIGEL_GP_SIG_GPTOURNAMENT_H

#include "compat/q2compat.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_GP/SIG_GPTournamentIndividual.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_GP/SIG_GPOperations.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

//#include "MT_Control/MT_Classifier.h"


class MT_Classifier;

namespace SIGEL_GP
{


/**
* This class is the parent class which all other GPTournament-classes will inherite from. It has a randomizer
* reference, for creation of random number in the genetic operator. It has an fitnesstrainer reference for 
* controlling the fitnesscomputation preformed by pvm. The GPTournament-class includes QVector for a datastructure
* of the envolved tournamentindividuals. The class has a reference of the actual individual pool, a flag for 
* indicating if the tournament is ready to play and an integer value, which shows how many tournament are to 
* play before this tournament can be played.
*/

class SIG_GPTournament
{ 
  /**
   * The current language parameter settings.
   */

 public:
 SIGEL_Robot::SIG_LanguageParameters &languageP;

  /**
   * The randomizer, which is needed to create the randompoint used for the genetic operations.
   */
 public:
SIGEL_Tools::SIG_Randomizer& randomizer;

  /**
   * The fitnesstrainer controls the computations of fitnessvalues after the genetic mutation of 
   * the winner of the tournament.
   */
 public:
SIG_GPFitnessTrainer& trainer;

  /**
   * The tournamentindividual is used record the positions of the tournament members and the
   * next tournament in which the tournamentindividual is member of, plus the taskid of the
   * pvm-task given from the fitnesstrainer.
   */
 public:
QList<SIG_GPTournamentIndividual *> indis;

  /**
   * The flag signals if the tournament can be played or have to wait for earlier tournaments.
   *
   */
 public:
bool justWaiting ;

  /**
   * The number of tournaments which have to be played first, before this tournament can be played.
   *
   */
 public:
int depNumber;

  /**
   * The actual individual pool.
   *
   */
 public:
SIG_GPPopulation &gpPool;

/**
 * The current GP settings that contain neccessary information for some GP operations.
 */

 public:
 SIG_GPParameter& gpParameter;


  /**
   * The constructor of the GPTournament.
   * @pre
   * A tournamentset is in creation.
   * @post
   * All attributes are set to the parameters an a topological sorting is expected.
   * @param randomizer
   * The reference to the randomizer object of the GPManager.
   * @param trainer
   * The reference to the GPFitnesstrainer of the GPManager.
   * @param actPool
   * The reference to the actual pool object.
   */
 public:
SIG_GPTournament(SIGEL_Tools::SIG_Randomizer& randomizer,
		 SIG_GPFitnessTrainer& trainer, 
		 SIG_GPPopulation& actPool,
		 SIG_GPParameter& gpParameter,
		 SIGEL_Robot::SIG_LanguageParameters &languageP);

  /**
   * The destructor of the tournament.
   * @pre
   * The tournament is played, the winner was the victim of cruel genetic operations and the fitness of
   * its debris was computed and the debris was placed in the pool for other demonic tournaments.
   * @post
   * The object is destructed.
   */
 public:
 virtual ~SIG_GPTournament();

  /**
   * This method is virtual, for definition in the inherited class.
   */
 public:
 virtual bool run();
 
 public:
	 virtual bool run(MT_Classifier * MetaClassifier);

 public:
 virtual bool classify(MT_Classifier * MetaClassifier);


 protected:
 void inhume( SIG_GPIndividual &corps );

};

}
#endif //  SIGEL_GP_SIG_GPTOURNAMENT_H
