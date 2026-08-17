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
#ifndef SIGEL_SIMULATION_SIG_DYNALINK_H
#define SIGEL_SIMULATION_SIG_DYNALINK_H

#include <qvector.h>
#include "constraint.h"
#include "ptp.h"
#include <SOLID/solid.h>
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"

namespace SIGEL_Simulation
{

/**
 * The SIG_DynaLink contains all the data of one link for Dynamo and Solid
 *
 * The class SIG_SimulationData transforms the SIG_Bodys and SIG_Links of the
 * SIG_Robot into SIG_DynaLinks, here everything is just stored plus there is
 * an easy function to add constraints and the callbacks are implemented.
 */
class SIG_DynaLink {
 public:
  DL_ptp * collptp;
  
  /** A pointer to the floor material, so we have the material in the static collision response */
  SIGEL_Robot::SIG_Material * floorMaterial;
 
  /** A pointer to the static link */
  SIGEL_Robot::SIG_Link const * link;

  /** number of the link */
  int number;
  
  /** position of the link's center of mass */
  DL_point position;

  /** orientation of the link as its local coordinate system */
  DL_matrix orientation;

  /** the representation of the link's physics in DyNamo */
  DL_dyna* dyna;

  /** the representation of the link's surface in SOLID */
  DtShapeRef shape;

  /** the constructor
   *
   * Adds itself to the here created dyna's companion
   */
  SIG_DynaLink(DL_point thePosition, DL_matrix theOrientation);

  /** the destructor
   *
   * Destroys the dyna
   */
  ~SIG_DynaLink();

  /** Needed to change a geo which is not controlled by DyNamo from outside
   *
   * @param g
   * the geo to be changed here
   */
  void getNewGeoInfo(DL_geo* g);

  /** Is called, when a Dyna has changed position or orientation
   *
   * changes the position and orientation stored here
   * @param d
   * the dyna, should be the same as dyna
   */
  void updateDynaCompanion(DL_dyna* d);

  /** Is called, when the companion is created
   *
   * wants the position and orientation to be set in g
   * @param g
   * the geo to be changed here
   */
  void getFirstGeoInfo(DL_geo* g);

  /** Needed to check the new InertiaTensor, if the Link is deformed
   *
   * Will not be implemented in the program, it is just there to
   * completely implement the callbacks from DyNamo
   * @param d
   * the dyna whose inertia tensor to be changed
   */
  void checkInertiaTensor(DL_dyna* d);
};

}

#endif // SIGEL_SIMULATION_SIG_DYNALINK_H

