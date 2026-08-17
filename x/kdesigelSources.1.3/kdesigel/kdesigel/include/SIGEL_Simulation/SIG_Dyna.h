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
#ifndef SIGEL_SIMULATION_SIG_DYNA_H
#define SIGEL_SIMULATION_SIG_DYNA_H

#include "dyna.h"
#include "SIGEL_Simulation/SIG_DynaJoint.h"

namespace SIGEL_Simulation
{
/** This class extends DynaMo's DL_dyna class
 */
class SIG_Dyna : public DL_dyna {
 public:
  /** constructor, does nothing but calling its parents constructor */
  SIG_Dyna(void* companion);
  /** destructor, does nothing but calling its parents destructor */
  ~SIG_Dyna();
  /** This methods calculates the Force in one local point
   * @param pd the point which force shall be read
   * @return the force in point pd
   */
  DL_vector getPointForce(DL_point * pd);
  DL_vector getForce();
  DL_vector getMomentum();
  DL_vector * get_inertiatensor();
};

};

#endif // SIGEL_SIMULATION_SIG_DYNA_H
