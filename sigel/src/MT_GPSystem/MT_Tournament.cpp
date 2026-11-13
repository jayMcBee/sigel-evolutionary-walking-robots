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
#include "MT_GPSystem/MT_Tournament.h"

MT_Tournament::MT_Tournament( int Size, int num )
{
	Individuals.resize( Size );
	for( int i = 0; i < Size; i++ )
	{
		Individuals[i] = nullptr;
	}

	NumberOfWinner = num;
	LastError = MT_ErrorCode::None;
	FirstFreePos = num;
	PopSize = Size;
	TypeOfIndividual = 0;
}

// further method

void MT_Tournament::setTypeOfIndividual( int Type )
{
	TypeOfIndividual = Type;
}

void MT_Tournament::ranking( QList<int> *WinnerLoser )
{
	int WinnerLoseSize = WinnerLoser->size();

	for( int i = 0; i < WinnerLoseSize; i++ )
	{
		( *WinnerLoser )[i] = 0;
	}

	double PresentFit = 0.0;
	double SmallestFit = 0.0;
	int PosSmallestFit = 0;

	QList<double> FitValue; // Gives the fitness of the individual at position Pos[i]
	FitValue.resize( NumberOfWinner );
	for( int i = 0; i < NumberOfWinner; i++ )
	{
		FitValue[i] = 0.0;
	}

	QList<int> Position; // Gives the individual's position in the population
	Position.resize( NumberOfWinner );
	for( int i = 0; i < NumberOfWinner; i++ )
	{
		Position[i] = 0;
	}

	// Find the NumberOfWinner fittest individuals
	for( int i = 0; i < PopSize; i++ )
	{
		PresentFit = Individuals[i]->getFitness();

		// Find the smallest fitness in FitValue and its position
		PosSmallestFit = 0;
		SmallestFit = FitValue[0];
		for( int k = 1; k < NumberOfWinner; k++ )
		{
			if( SmallestFit > FitValue[k] )
			{
				PosSmallestFit = k;
				SmallestFit = FitValue[k];
				if( SmallestFit == 0.0 )
				{
					break;
				}
			}
		}

		if( PresentFit > SmallestFit )
		{
			FitValue[PosSmallestFit] = PresentFit;
			Position[PosSmallestFit] = i;
		}
	}

	for( int k = 0; k < NumberOfWinner; k++ )
	{
		( *WinnerLoser )[Position[k]] = 1;
	}
}

void MT_Tournament::fitnessProp( MT_Randomizer *Randi, QList<int> *WinnerLoser )
{
	int WinnerLoseSize = WinnerLoser->size();
	for( int i = 0; i < WinnerLoseSize; i++ )
	{
		( *WinnerLoser )[i] = 0;
	}

	int Winner = 0;
	double FitTemp;

	QList<int> ProporFit;
	ProporFit.resize( PopSize );

	if( TypeOfIndividual == 0 ) // Evaluator Meta System;
	{
		for( int i = 0; i < PopSize; i++ )
		{
			FitTemp = 100.0 * Individuals[i]->getFitness();
			ProporFit[i] = FitTemp;
			if( ProporFit[i] <= 0 )
			{
				ProporFit[i] = 1;
			}
		}
	}
	else // Classifier Meta System;
	{
		for( int i = 0; i < PopSize; i++ )
		{
			ProporFit[i] = Individuals[i]->getFitness();
			ProporFit[i] = ProporFit[i] - 500;
			if( ProporFit[i] <= 0 )
			{
				ProporFit[i] = 1;
			}
		}
	}

	// Determine the winners using the randomizer
	for( int i = 0; i < NumberOfWinner; i++ )
	{
		Winner = Randi->getProportionalWinner( &ProporFit );
		( *WinnerLoser )[Winner] = 1;

		// A winner may not take part in any further tournament
		ProporFit[Winner] = 0;
	}
}
