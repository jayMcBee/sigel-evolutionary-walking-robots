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
#ifndef SIGEL_GP_SIG_GPSIMPLETOURNAMENT_H
#define SIGEL_GP_SIG_GPSIMPLETOURNAMENT_H

#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPFitnessTrainer.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_GP/SIG_GPTournament.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"


namespace SIGEL_GP
{

/**
* This class defineds the tournament with a reproduction of the winner as the tournament price. It uses
* the randomizer and the fitnesstrainer for randompoint creation and fitnessvalue computation.
* It inherits from SIG_GPTournament, so see there for further details.
*/

class SIG_GPSimpleTournament : public SIGEL_GP::SIG_GPTournament

{ 

/**
* The constructor of the Simpletournament obejct.
* @pre
* A tournamentset is in creation.
* @post
* The object is created initialized with the parameters an waits for the topological sorting and the 
* execution of the tournament
* @param randomizer
* The reference to the randomizer object of the GPManager.
* @param trainer
* The reference to the fitnesstrainer object of the GPManager.
* @param actPool
* The actual population of the pool.
* @param ppos1
* The first tournament member adress in the pool.
* @param ppos2
* The second tournament member adress in the pool.
*/
 public:
SIG_GPSimpleTournament( SIGEL_Tools::SIG_Randomizer& randomizer,
		        SIG_GPFitnessTrainer& trainer, 
		        SIG_GPPopulation& actPool, 
			SIG_GPParameter& gpParameter,
			SIGEL_Robot::SIG_LanguageParameters &languageP,
		        int ppos1, 
		        int ppos2 );

/**
* The destructor of the SimpleTournament object.
* @pre
* The tournament is played, the winner is reproducted, set to the pool and the next tournament wants to be played.
* @post
* The object is destructed.
*/
 public:
virtual ~SIG_GPSimpleTournament();

/**
* This virtual method executes the tournament.
*/
 public:
virtual bool run();


/**
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


private:
 QString name;

};

}
#endif //  SIGEL_GP_SIG_GPSIMPLETOURNAMENT_H


 //********************META changeen**************************************************
  /*two new methods: 
	bool runMetaClassifier(*Classifier) 
		-> runs a tournament without exact fitness, using the MetaClassifier!
		requires Classifier for double classifier (*SigProgOne, SigProgTwo)
 
	bool runMetaCorrect(*Classifier)
		-> serves to create new training cases!
		createNewTCase(*SigOne,SigTwo)


  */
