/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef MT_GPSYSTEM_MT_GENESISTYPE_H
#define MT_GPSYSTEM_MT_GENESISTYPE_H

/**
 * How the program of a MetaGP individual was created. The number is saved with
 * the individual, so each value keeps its number. A mutated individual saves
 * Mutation plus its number of mutated elements.
 */
enum class MT_GenesisType
{
	Undefined = -1,
	Random = 0,
	OnePointCrossover = 1,
	TwoPointCrossover = 2,
	ThreePointCrossover = 3,
	Reproduction = 4,
	CarriedOverParent = 5,
	Mutation = 100
};

#endif // MT_GPSYSTEM_MT_GENESISTYPE_H
