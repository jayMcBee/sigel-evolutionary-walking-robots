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
#include "SIGEL_Tools/SIG_Exception.h"

SIGEL_Tools::SIG_Randomizer::SIG_Randomizer()
{
  setNewSeed( 0 );
}

SIGEL_Tools::SIG_Randomizer::SIG_Randomizer(int seed)
{
  setNewSeed( seed );
}

void SIGEL_Tools::SIG_Randomizer::setNewSeed(int seed)
{
  if (seed == 0)
    engine.seed( std::random_device()() );
  else
    engine.seed( seed );
}

int SIGEL_Tools::SIG_Randomizer::getRandomInt(int maximum)
{
  return static_cast<int>( getRandomLong( maximum ) );
}

long SIGEL_Tools::SIG_Randomizer::getRandomLong(long maximum)
{
  if (maximum < 0)
    throw SIG_Exception( __FILE__, __LINE__, "A random number below a negative maximum was asked for." );

  if (maximum == 0)
    return 0;

  return std::uniform_int_distribution<long>( 0, maximum - 1 )( engine );
}
