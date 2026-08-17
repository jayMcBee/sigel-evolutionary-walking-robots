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
#ifndef SIGEL_SIMULATION_SIG_DYNASENSOR_H
#define SIGEL_SIMULATION_SIG_DYNASENSOR_H

#include "pointvector.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Simulation/SIG_DynaJoint.h"

namespace SIGEL_Simulation
{
/** This class is the dynamic representation of SIG_Sensor
 *
 * it is only used because every sensor has a special number
 */
class SIG_DynaSensor {
 public:
  /** A pointer to the static sensor */
  SIGEL_Robot::SIG_Sensor const * sensor;
  /** The number of the sensor */ 
  int number;
  /** The joint */
  SIG_DynaJoint * joint;
  /** Sense the joints first deflection */
  DL_Scalar senseJoint1();
  /** Sense the joints first deflection (cylindrical only) */
  DL_Scalar senseJoint2();
};

};

#endif // SIGEL_SIMULATION_SIG_DYNASENSOR_H
