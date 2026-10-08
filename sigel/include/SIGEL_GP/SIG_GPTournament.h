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

#include <QList>
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPTournamentIndividual.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_GP/SIG_GPOperations.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

class MT_Classifier;

namespace SIGEL_GP
{


	/**
	 * This class is the parent class which all other GPTournament-classes will inherite from. It has a randomizer
	 * reference, for creation of random number in the genetic operator. The GPTournament-class includes QList
	 * for a datastructure of the envolved tournamentindividuals. The class has a reference of the actual individual
	 * pool, a flag for indicating if the tournament is ready to play and an integer value, which shows how many of
	 * its individuals are still in an earlier tournament.
	 */

	class SIG_GPTournament
	{
	public:
		/**
		 * The constructor of the GPTournament.
		 * @pre
		 * A tournamentset is in creation.
		 * @post
		 * All attributes are set to the parameters an a topological sorting is expected.
		 * @param randomizer
		 * The reference to the randomizer object of the GPManager.
		 * @param actPool
		 * The reference to the actual pool object.
		 */
		SIG_GPTournament(SIGEL_Tools::SIG_Randomizer& randomizer,
		                 SIG_GPPopulation& actPool,
		                 SIG_GPParameter& gpParameter,
		                 SIGEL_Robot::SIG_LanguageParameters &languageParameters);

		/**
		 * The destructor of the tournament.
		 * @pre
		 * The tournament is played, the winner was the victim of cruel genetic operations and the fitness of
		 * its debris was computed and the debris was placed in the pool for other demonic tournaments.
		 * @post
		 * The object is destructed.
		 */
		virtual ~SIG_GPTournament();

		/**
		 * This method is virtual, for definition in the inherited class.
		 */
		virtual bool run();

		virtual bool run(MT_Classifier * MetaClassifier);

		virtual bool classify(MT_Classifier * MetaClassifier);

		/**
		 * The current language parameter settings.
		 */
		SIGEL_Robot::SIG_LanguageParameters &languageParameters;

		/**
		 * The randomizer, which is needed to create the randompoint used for the genetic operations.
		 */
		SIGEL_Tools::SIG_Randomizer& randomizer;

		/**
		 * The tournamentindividual is used record the positions of the tournament members and the
		 * next tournament in which the tournamentindividual is member of, plus the taskid of the
		 * pvm-task given from the fitnesstrainer.
		 */
		QList<SIG_GPTournamentIndividual *> indis;

		/**
		 * The flag signals if the tournament can be played or have to wait for earlier tournaments.
		 *
		 */
		bool justWaiting ;

		/**
		 * The number of individuals of this tournament that are still in an earlier tournament.
		 * The tournament can be played when it is 0.
		 */
		int waitCounter;

		/**
		 * The actual individual pool.
		 *
		 */
		SIG_GPPopulation &gpPool;

		/**
		 * The current GP settings that contain neccessary information for some GP operations.
		 */
		SIG_GPParameter& gpParameter;

	protected:
		void inhume( SIG_GPIndividual &corps );
	};

}
#endif //  SIGEL_GP_SIG_GPTOURNAMENT_H
