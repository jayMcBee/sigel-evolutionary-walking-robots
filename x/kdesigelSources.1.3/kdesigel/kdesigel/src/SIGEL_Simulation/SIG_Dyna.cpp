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
#include "SIGEL_Simulation/SIG_Dyna.h"

SIGEL_Simulation::SIG_Dyna::SIG_Dyna(void* companion) :
  DL_dyna(companion)
{
};

SIGEL_Simulation::SIG_Dyna::~SIG_Dyna()
{
};

DL_vector SIGEL_Simulation::SIG_Dyna::getPointForce(DL_point * pd)
{
  if ((!Muptodate)||(!Fuptodate))
   integrate();
   
  DL_vector d,dw,Fp,minM;
  pd->tovector(&d);
  to_world(&d,&dw);
  M.times(-1,&minM);
  minM.crossprod(&dw,&Fp);
  Fp.plusis(&F);
  //  Fp.times(-1,&Fp);
  return Fp;
};


DL_vector SIGEL_Simulation::SIG_Dyna::getForce()
{
 if (!Fuptodate)
  integrate();
 return F;
};

DL_vector SIGEL_Simulation::SIG_Dyna::getMomentum()
{
 if (!Muptodate)
  integrate();
 return M;
};


DL_vector * SIGEL_Simulation::SIG_Dyna::get_inertiatensor()
{
 return &J;
};



