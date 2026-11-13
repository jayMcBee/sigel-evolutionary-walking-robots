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
#ifndef MT_GPSYSTEM_MT_TOURNAMENT_H
#define MT_GPSYSTEM_MT_TOURNAMENT_H

#include "MT_GPSystem/MT_Population.h"


/* This class represent a single GP tournament;
* it inherited from theMT_Population
* it determine the tournament winner (s)
*/
class MT_Tournament : public MT_Population
{
public:
	/* realization of a fitness proportional selection
	* @pre: the tournament member must be inside the tournament
	* @pre: the fitness of the tournament member must be estimated beforehand
	* @post: in the WinnerLoser array indicat a one a Winner and a zero a Loser
	*/
	void fitnessProp( MT_Randomizer *Randi, QList<int> *WinnerLoser );

	/* realization of a ranking selection
	* @pre: the tournament member must be inside the tournament
	* @pre: the fitness of the tournament member must be estimated beforehand.
	* @post: in the WinnerLoser array indicat a one a Winner and a zero a Loser
	*/
	void ranking( QList<int> *WinnerLoser );

	void setTypeOfIndividual( int Type );

	/* create a empty tournament of Size "Size" and a "Winner Array" */
	MT_Tournament( int Size, int num );
	MT_Tournament() = default;

private:
	/* the number of winner in this tournament
	* @pre: NumberOfWinner<= tournament.Size
	*/
	int NumberOfWinner;

	int TypeOfIndividual;

	/*  denote which players have gain the present tournaments and which are the loser*/
	QList<int> *WinnerLoser;
};

#endif // MT_GPSYSTEM_MT_TOURNAMENT_H
