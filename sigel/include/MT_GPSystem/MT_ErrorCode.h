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
#ifndef MT_GPSYSTEM_MT_ERRORCODE_H
#define MT_GPSYSTEM_MT_ERRORCODE_H

/**
 * The last error of a MetaGP class, read through its getLastError(). The codes
 * pass between classes: MT_GPManager takes over the codes of its parts.
 */
enum class MT_ErrorCode
{
	None = 0,

	// MT_Randomizer
	RandomizerSectionMissing = 260,
	ConstantSectionMissing = 261,
	RandomizerSetupSectionMissing = 262,
	ConstantSetupSectionMissing = 263,

	// MT_Population
	ImportPopulationSectionMissing = 320,
	PopulationSectionMissing = 321,
	PopulationFull = 340,

	// MT_Program
	ProgramSectionMissing = 360,

	// MT_Individual
	IndividualSectionMissing = 370,

	// MT_Search
	BroodSizeSectionMissing = 410,

	// MT_FitnessTrainer
	FitnessTrainerSectionMissing = 601,
	FitnessTrainerSetupSectionMissing = 602,
	UnknownFitnessFunction = 610
};

#endif // MT_GPSYSTEM_MT_ERRORCODE_H
