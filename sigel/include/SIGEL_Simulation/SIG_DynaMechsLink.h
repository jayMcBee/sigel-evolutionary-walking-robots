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
#ifndef SIGEL_SIMULATION_SIG_DYNAMECHSLINK_H
#define SIGEL_SIMULATION_SIG_DYNAMECHSLINK_H

#include "SIGEL_Robot/SIG_Link.h"

#include <QList>

#include <dm.h>
#include <dmRigidBody.hpp>
#include <dmZScrewTxLink.hpp>


namespace SIGEL_Simulation
{

class SIG_DynaMechsLink
  {

  public:

    SIG_DynaMechsLink( int dynaMechsLinkNumber,
		       SIGEL_Robot::SIG_Link const *link,
		       dmRigidBody *dynaMechsLink,
		       double screwD,
		       double screwTheta );

    int const dynaMechsLinkNumber;

    SIGEL_Robot::SIG_Link const * const link;

    double const screwD;

    double const screwTheta;

    dmZScrewTxLink *screwLink;

    dmRigidBody * const dynaMechsLink;

    NEWMAT::Matrix transformation;

    QList< SIG_DynaMechsLink * > successors;

    void forwardKinematics( SIG_DynaMechsLink *caller );

    NEWMAT::Matrix buildXRotationMatrix( double angle );

    NEWMAT::Matrix buildZRotationMatrix( double angle );

    NEWMAT::Matrix buildTranslationMatrix( double x,
					   double y,
					   double z );
  };

}

#endif // SIGEL_SIMULATION_SIG_DYNAMECHSLINK_H
