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
#ifndef SIGEL_GP_SIG_GPOPERATIONS_H
#define SIGEL_GP_SIG_GPOPERATIONS_H

#include <QList>
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include <qstring.h>


namespace SIGEL_GP
{

/**
* This class contains the genetic operations for the genetic algorithms. The operations
* are declared as static functions, so no attributes are there. The needed data have to
* be given at the function call as parameters.
*/

class SIG_GPOperations
{

/**
* This operation performes crossover of the robot control programs
* of two individuals. Via variations 1 point as well as 2 point crossover
* is applied to the selected individuals.
* @pre
* A two tournaments has happened and the winners have to crossover.
* @post
*  The two individuals carrying a new combination of a robot control program are created and
* the parent individuals are unmodified.
* @param winner1
* The involved individual number one.
* @param name1
* The name of the offspring of winner1.
* @param poolPos1
* The position in the pool, where the offspring of winner1 is to put.
* @param winner2
* The involved individual number two.
* @param name2
* The name of the offspring of winner2.
* @param poolPos2
* The position in the pool, where the offspring of winner2 is to put.
* @param randomizer
* The randomizer is used for creation of random crossoverpoints.
* @return
* The offspring is returned as complete individuals in a Qvector.
*/
 public:
  static QList<SIGEL_GP::SIG_GPIndividual *> crossOver( SIGEL_GP::SIG_GPIndividual& winner1,
							int poolPos1,
							SIG_GPIndividual& winner2,
							int poolPos2,
							QString newName1,
							QString newName2,
							SIGEL_Tools::SIG_Randomizer& randomizer,
							SIGEL_GP::SIG_GPParameter& gpParameter,
							SIGEL_Robot::SIG_LanguageParameters &languageP );

/** This Operation mutate the Individual
* @pre
* A tournament was played, with mutation for the winner.
* @post
* A mutated copy of winner is created.
* @param  winner 
* The individual, which won the tournament.
* @param name
* The name of the offspring.
* @param poolPos
* The position of the new individual in the pool.
* @param randomizer
* The randomizer is used for creation of a random mutationpoint.
* @return
* The offspring of winner as a complete individual.
*/
 public:
  static SIGEL_GP::SIG_GPIndividual& mutation( SIGEL_GP::SIG_GPIndividual& winner,
					       int poolpos,
					       QString newName,
					       SIGEL_Tools::SIG_Randomizer& randomizer,
					       SIGEL_GP::SIG_GPParameter& gpParameter,
					       SIGEL_Robot::SIG_LanguageParameters &languageP);

/** This operation gets the winner of an played tournament with reproduction. 
* It returns the same Individual without any changes.
* @pre
* A tournament was played, with reproduction as the genetic operator.
* @post
* The offspring, a mutated copy of winner, is created.
* @param winner
* The winner of the tournament.
* @param name
* The name of the offspring.
* @param poolPos
* The position of the new individual in the pool.
* @return
* The offspring of winner as a pointer of complete individual.
*/
 public:
  static SIGEL_GP::SIG_GPIndividual& reproduction(SIGEL_GP::SIG_GPIndividual& winner,
						  QString newName,
						  int poolPos,
						  SIGEL_Tools::SIG_Randomizer& randomizer,
						  SIGEL_GP::SIG_GPParameter& gpParameter,
						  SIGEL_Robot::SIG_LanguageParameters &languageP );

};

}
#endif //  SIGEL_GP_SIG_GPOPERATIONS_H
