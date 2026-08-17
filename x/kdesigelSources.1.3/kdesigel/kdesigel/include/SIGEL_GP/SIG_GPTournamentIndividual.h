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
#ifndef SIGEL_GP_SIG_GPTOURNAMENTINDIVIDUAL_H
#define SIGEL_GP_SIG_GPTOURNAMENTINDIVIDUAL_H

namespace SIGEL_GP
{

/**
* In this class the special individuals for the genetic programming tournament is defined. For the GPManager is it
* important to know, which tournament is to play next. This is not a simple question because of the 
* steady-state-demand. The steady-state-demand means, that for every tournament is every combination of individuals
* possible. So when the tournaments set for the next generation is computed, it will be sorted topological, in
* reference to the dependencies between the tournaments. Therefor it is important to know, that the positions in the
* tournaments means positions in the pool, not individuals itself. A tournamentindividual which want to play a 
* tournament, have to play all other earlier tournaments first in which it is envolved. To show the dependencies,
* every tournamentindividual knows it next tournament, in which it is envolved. If a fitnesscomputation is in progress
* the individual knows its taskid to receive the fitnessvalue after the computation.
*/

class SIG_GPTournamentIndividual
{ 
	/**
	* The indentifier of the individual given as an integer.
	*/
 public:
int indNumber;
	
	/**
	* The identifier of the next tournament, the individual is envolved in.
	*/
 public:
int successor ;

	/**
	* The taskID of the pvm task, which runs the simulation run.
	*/
 public:  
int fitTaskId ;

	/**
	* The constructor of a tournamentindividual.
	* @pre
	* The tournamentset is in creation and a tournamentindividual is needed.
	* @post
	* The tournamentindividual is created, the parameter is set to the indnumber attribute, 
	* all other attributes are set to zero.
	* @param tind
	* The identifier of the tournamentindividual.
	*/
 public:
SIG_GPTournamentIndividual(int tind);
};

}
#endif //  SIGEL_GP_SIG_GPTOURNAMENTINDIVIDUAL_H
