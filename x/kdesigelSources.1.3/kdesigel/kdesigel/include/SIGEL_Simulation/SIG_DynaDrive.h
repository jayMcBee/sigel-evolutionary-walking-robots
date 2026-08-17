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
#ifndef SIGEL_SIMULATION_SIG_DYNADRIVE_H
#define SIGEL_SIMULATION_SIG_DYNADRIVE_H

#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Simulation/SIG_DynaJoint.h"

namespace SIGEL_Simulation
{
/** This class is the dynamic representation of SIG_Drive
 *
 * it is only used because every drive has special number
 */
class SIG_DynaDrive {
 public:
  /** A pointer to the static joint */
  SIGEL_Robot::SIG_Drive const * drive;
  /** The number of the drive */ 
  int number;
  /** The Joint */
  SIG_DynaJoint * joint;
  /** Apply forces with length f1,f2 */
  void applyForce(DL_Scalar f1, DL_Scalar f2);
};

};

#endif // SIGEL_SIMULATION_SIG_DYNADRIVE_H
