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
#ifndef SIGEL_SIMULATION_SIG_DYNACALLBACKS_H
#define SIGEL_SIMULATION_SIG_DYNACALLBACKS_H

namespace SIGEL_Simulation { class SIG_DynaSystem; };

#include "dyna_system.h"
#include <qstring.h>
#include "SIGEL_Simulation/SIG_DynaSystem.h"

namespace SIGEL_Simulation
{

/**
 * The SIG_DynaCallbacks implements the callback-functions of Dynamo
 *
 * Since the callbacks have to be handled either in the dyna's/geo's
 * companion (which is here the SIG_DynaLink) or in our SIG_DynaSystem,
 * the class needs two function pointers.
 */
class SIG_DynaCallbacks : public DL_dyna_system_callbacks {

 public:

  /** Storage for the Msg-function pointer
   */
  SIG_DynaSystem* dynaSystem;

  /** the constructor
   */
  SIG_DynaCallbacks();


 protected:
  
  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaLink
   */
  virtual void get_new_geo_info(DL_geo* theGeo);

  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaLink
   */
  virtual void update_dyna_companion(DL_dyna* theDyna);

  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaLink
   */
  virtual void get_first_geo_info(DL_geo* theGeo);

  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaLink
   */
  virtual void check_inertiatensor(DL_dyna* theDyna);

  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaSystem
   */
  virtual void do_collision_detection();

  /** consult the DyNamo Documentation Ch.4 Installation for more details
   *
   * redirects to SIG_DynaSystem
   */
  virtual void Msg(char* message, ...);

};

}

#endif // SIGEL_SIMULATION_SIG_DYNACALLBACKS_H
