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
#ifndef MT_GPSYSTEM_MT_SEARCH_H
#define MT_GPSYSTEM_MT_SEARCH_H

#include <QTextStream>
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_Randomizer.h"
#include "MT_GPSystem/MT_ErrorCode.h"


/* This class manage the GP variation;
* another search points in the search space were determinate.
* the offspring will new create from the parent.
*/
class MT_Search
{
public:

	/*start the search process, it's determinate or
	* create new individuals with regard to the search parameters.
	* @pre: there are a corrcet parent, offspring population
	* @pre: there are a correct randinomizer, which have the necessary
	* search parameters
	* @param: return value indicate an error; MT_ErrorCode::None := it was a correct search
	*/
	MT_ErrorCode startMatingProcess();

	/* set/get functions used to update the GUI or the GP system*/
	void setBrutSize( int SizeOfBrut );
	int getBrutSize();
	MT_ErrorCode getLastError();

	MT_Search( MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * NewRandi, QTextStream & File );
	MT_Search( MT_Population * ParentPop, MT_Population * OffspringPop, MT_Randomizer * NewRandi );
	MT_Search() = default;
	virtual ~MT_Search() = default;

private:

	/* generate a exact copy of the individuals*/
	MT_Individual * reproduce( MT_Individual * Progenitor );

	/* generate two new individuals form two parents */
	void crossover( MT_Individual *ParentOne, MT_Individual *ParentTwo );

	/* generate a mutated copy from a parent*/
	MT_Individual * mutate( MT_Individual * Progenitor );

	/*pointer of the offspring population*/
	MT_Population * TargetPop;

	/* pointer of the parent*/
	MT_Population * SourcePop;
	MT_Randomizer * Randi;

	// The brood size. It is loaded and saved; no search reads it.
	int BrutSize;
	MT_ErrorCode LastError;

	MT_Individual * ChildOne;
	MT_Individual * ChildTwo;
	MT_Individual * Parent;
	MT_Individual * FirstXOverParent;
};

#endif // MT_GPSYSTEM_MT_SEARCH_H
