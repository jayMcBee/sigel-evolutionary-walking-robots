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
#ifndef SIGEL_SIMULATION_SIG_TRANSLATIONALCONTROLLER_H
#define SIGEL_SIMULATION_SIG_TRANSLATIONALCONTROLLER_H

#include "controller.h"
#include "SIGEL_Simulation/SIG_DynaJoint.h"

namespace SIGEL_Simulation
{
/** This class is a child of DynaMo's DL_controller class
 *
 * it is used to control the minimal and the maximal deflection of the joint
 */
class SIG_TranslationalController : public DL_controller {
  /** storage of the minimum deflection */
  DL_Scalar minimum;
  /** storage of the maximum deflection */
  DL_Scalar maximum;
  /** storage of the joint */
  SIG_DynaJoint * joint;
 public:
  /** constructor, does nothing but calling its parents constructor */
  SIG_TranslationalController();
  /** destructor, does nothing but calling its parents destructor */
  ~SIG_TranslationalController();
  /** This method initializes the controller
   * @param theJoint is a link to the Joint which deflection shall be controlled
   * @param theMinimum is the minimal amount of deflection
   * @param theMaximum is the maximal amount of deflection
   * @post the controller is activated in Dynamo
   */
  void init(SIG_DynaJoint * theJoint, DL_Scalar theMinimum, DL_Scalar theMaximum);
  /** This is the method DynaMo calls for the controller calculation */
  virtual void calculate_and_apply(); 
};

};

#endif // SIGEL_SIMULATION_SIG_TRANSLATIONALCONTROLLER_H
