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
#include "SIGEL_Simulation/SIG_DynaCallbacks.h"
#include "SIGEL_Simulation/SIG_DynaLink.h"
#include "SIGEL_Tools/SIG_IO.h"

SIGEL_Simulation::SIG_DynaCallbacks::SIG_DynaCallbacks()
{
  dynaSystem=0;
};

void SIGEL_Simulation::SIG_DynaCallbacks::get_new_geo_info(DL_geo* theGeo)
{
  ((SIG_DynaLink*)(theGeo->get_companion()))->getNewGeoInfo(theGeo);
};

void SIGEL_Simulation::SIG_DynaCallbacks::update_dyna_companion(DL_dyna* theDyna)
{
  ((SIG_DynaLink*)(theDyna->get_companion()))->updateDynaCompanion(theDyna);
};

void SIGEL_Simulation::SIG_DynaCallbacks::get_first_geo_info(DL_geo* theGeo)
{
  ((SIG_DynaLink*)(theGeo->get_companion()))->getFirstGeoInfo(theGeo);
};

void SIGEL_Simulation::SIG_DynaCallbacks::check_inertiatensor(DL_dyna* theDyna)
{
  ((SIG_DynaLink*)(theDyna->get_companion()))->checkInertiaTensor(theDyna);
};

void SIGEL_Simulation::SIG_DynaCallbacks::do_collision_detection()
{
  if (dynaSystem!=0)
    dynaSystem->doCollisionDetection();
};

void SIGEL_Simulation::SIG_DynaCallbacks::Msg(char* message, ...)
{
#ifdef SIG_DEBUG
 SIGEL_Tools::SIG_IO::cerr << message; 
#endif
  if (dynaSystem!=0)
    dynaSystem->msgFunction(QString(message));
};

