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
#include "SIGEL_Simulation/SIG_DynaLink.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include "SIGEL_Simulation/SIG_DynaSystem.h"
#include "SIGEL_Tools/SIG_IO.h"
#include <qarray.h>
#ifdef _WINDOWS
#include <float.h>
#endif
#include "NaN.h"

SIGEL_Simulation::SIG_DynaLink::SIG_DynaLink(DL_point thePosition, DL_matrix theOrientation)
{
  position=thePosition;
  orientation.assign(&theOrientation);
/*  SIGEL_Tools::SIG_IO::cerr << "Orient " << orientation.c0.x
                               << " " << orientation.c1.x
                               << " " << orientation.c2.x << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << orientation.c0.y
                               << " " << orientation.c1.y
                               << " " << orientation.c2.y << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << orientation.c0.z
                               << " " << orientation.c1.z
                               << " " << orientation.c2.z << "\n";*/
  collptp=0;
  dyna = new SIG_Dyna((void*)this);
/*  DL_vector v(0,0,0);
  dyna->set_next_orientation(&orientation);
  dyna->set_next_position(&position);
  dyna->set_next_velocity(&v);
  dyna->move(&position,&orientation);
  dyna->set_next_angvelocity(&v);
*/
  dyna->set_position(&position);
  dyna->set_orientation(&orientation);
/*  SIGEL_Tools::SIG_IO::cerr << "Orient " << dyna->get_orientation()->c0.x
                               << " " << dyna->get_orientation()->c1.x
                               << " " << dyna->get_orientation()->c2.x << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << dyna->get_orientation()->c0.y
                               << " " << dyna->get_orientation()->c1.y
                               << " " << dyna->get_orientation()->c2.y << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << dyna->get_orientation()->c0.z
                               << " " << dyna->get_orientation()->c1.z
                               << " " << dyna->get_orientation()->c2.z << "\n";*/
/*dyna->set_velocity(&v);
  dyna->set_angvelocity(&v);*/
#ifdef SIG_DEBUG  
  SIGEL_Tools::SIG_IO::cerr << " ************ DynaLink **********************************\n";
  SIGEL_Tools::SIG_IO::cerr << "Dyna: " << dyna << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Masse: " << dyna->get_mass() << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Inertia X:" << static_cast<SIG_Dyna*>(dyna)->get_inertiatensor()->x
                               << " Y:" << static_cast<SIG_Dyna*>(dyna)->get_inertiatensor()->y
                               << " Z:" << static_cast<SIG_Dyna*>(dyna)->get_inertiatensor()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Vel X:" << dyna->get_velocity()->x
                               << " Y:" << dyna->get_velocity()->y
                               << " Z:" << dyna->get_velocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "AVel X:" << dyna->get_angvelocity()->x
                                << " Y:" << dyna->get_angvelocity()->y
                                << " Z:" << dyna->get_angvelocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Pos X:" << dyna->get_position()->x
                               << " Y:" << dyna->get_position()->y
                               << " Z:" << dyna->get_position()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Orient " << dyna->get_orientation()->c0.x
                               << " " << dyna->get_orientation()->c1.x
                               << " " << dyna->get_orientation()->c2.x << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << dyna->get_orientation()->c0.y
                               << " " << dyna->get_orientation()->c1.y
                               << " " << dyna->get_orientation()->c2.y << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << dyna->get_orientation()->c0.z
                               << " " << dyna->get_orientation()->c1.z
                               << " " << dyna->get_orientation()->c2.z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next Vel X:" << dyna->get_next_velocity()->x
                               << " Y:" << dyna->get_next_velocity()->y
                               << " Z:" << dyna->get_next_velocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next AVel X:" << dyna->get_next_angvelocity()->x
                                << " Y:" << dyna->get_next_angvelocity()->y
                                << " Z:" << dyna->get_next_angvelocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next Pos X:" << dyna->get_next_position()->x
                               << " Y:" << dyna->get_next_position()->y
                               << " Z:" << dyna->get_next_position()->z << "\n";
#endif
};

SIGEL_Simulation::SIG_DynaLink::~SIG_DynaLink()
{
  delete dyna;
  if (collptp!=0)
   delete collptp;
};

void SIGEL_Simulation::SIG_DynaLink::getNewGeoInfo(DL_geo* g)
{
/*  DL_vector v;
  v=(g->get_velocity());
  if (isnan(v.x)) v.x=0;
  if (isnan(v.y)) v.y=0;
  if (isnan(v.z)) v.z=0;
  g->set_angvelocity(&v);
  v=(g->get_angvelocity());
  if (isnan(v.x)) v.x=0;
  if (isnan(v.y)) v.y=0;
  if (isnan(v.z)) v.z=0;
  g->set_angvelocity(&v);
  g->move(&position,&orientation);*/
  g->set_position(&position);
  g->set_orientation(&orientation);
};

void SIGEL_Simulation::SIG_DynaLink::updateDynaCompanion(DL_dyna* d)
{
  position.assign(d->get_position());
  orientation.assign(d->get_orientation());
  
  // SOLID updaten (fuer naechsten Frame)
  DL_point nextPosition;
  nextPosition.assign(d->get_next_position());
  DL_matrix nextOrientation;
  nextOrientation.assign(d->get_next_orientation());
  SIG_DynaSystem::unNaN(nextOrientation);
  DL_vector pos2;
  nextPosition.tovector(&pos2);
  dtSelectObject(this);
  dtLoadIdentity();
  QArray<double> rotmat(16);
  for (int j=0; j<3; j++)
    rotmat[ (j * 4) + 3 ] = 0;
    rotmat[ 15 ] = 1; 
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      rotmat[ i + (4 * j) ] = nextOrientation.get(i,j);  
  for (int k=0; k<3; k++)
    rotmat[ 12 + k ] = pos2.get(k);
  dtMultMatrixd(rotmat.data());
};

void SIGEL_Simulation::SIG_DynaLink::getFirstGeoInfo(DL_geo* g)
{
  getNewGeoInfo(g);
  // hier koennten spaeter initiale Geschwindigkeiten eingestellt werden.
};

void SIGEL_Simulation::SIG_DynaLink::checkInertiaTensor(DL_dyna* d)
{
  // Wird noch nicht implementiert, wird nur fuer Verformungen benoetigt
};
