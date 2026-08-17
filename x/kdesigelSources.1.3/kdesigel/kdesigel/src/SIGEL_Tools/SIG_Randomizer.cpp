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
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <qdatetime.h>

SIGEL_Tools::SIG_Randomizer::SIG_Randomizer()
  : next( 0 )
{
  setNewSeed( 0 );
};

SIGEL_Tools::SIG_Randomizer::SIG_Randomizer(int seed)
  : next( 0 )
{
  setNewSeed( seed );
};

void SIGEL_Tools::SIG_Randomizer::setNewSeed(int seed)
{
  if (seed == 0)
    {
      QTime midNight( 0, 0 );
      next = midNight.secsTo( QTime::currentTime() );
    }
  else
    next = seed;
}

SIGEL_Tools::SIG_Randomizer::~SIG_Randomizer()
{ };

int SIGEL_Tools::SIG_Randomizer::getRandomInt(int maximum)
{
  next = next*1103515245 + 12345;
  int randomNumber = static_cast<unsigned int>( next / 65536 ) % 32768;

  if (maximum == 0)
    return 0;
  else
    return randomNumber % maximum;
}

long SIGEL_Tools::SIG_Randomizer::getRandomLong(long maximum)
{
  return getRandomInt( static_cast<int>(maximum) );
}




