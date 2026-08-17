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
#ifndef SIGEL_SIMULATION_SIG_DYNAJOINT_H
#define SIGEL_SIMULATION_SIG_DYNAJOINT_H

#include "constraint.h"
#include "controller.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Simulation/SIG_DynaLink.h"

namespace SIGEL_Simulation
{
/** This class is the dynamic representation of SIG_Joint
 *
 * it is only used because every joint has special number
 */
class SIG_DynaJoint {
 public:
  /** The destructor */
  ~SIG_DynaJoint();
  /** A pointer to the static joint */
  SIGEL_Robot::SIG_Joint const * joint;
  /** The number of the joint */ 
  int number;
  /** The constraint in Dynamo */
  DL_constraint* constraint;
  /** The constraint for the maximal deflection */
  DL_constraint* deflection;
  /** The vectors for Dynamo */
  DL_vector leftDir,rightDir,leftUp,rightUp;
  /** The points for Dynamo */
  DL_point leftFix,rightFix,leftFixB,rightFixB,leftFixC,rightFixC;
  /** The two Links */
  SIG_DynaLink const * leftDyna;
  SIG_DynaLink const * rightDyna;
  /** The controller for the maximal deflection */
  DL_controller * controller;
  /** A second controller for Cylindrical Joints */
  DL_controller * controller2;
  /** Is this a deflection based drive? */
  bool holdDeflection;
  /** The momentary deflection for deflection based drives */
  DL_Scalar momentaryDeflection;
};

};

#endif // SIGEL_SIMULATION_SIG_DYNAJOINT_H
