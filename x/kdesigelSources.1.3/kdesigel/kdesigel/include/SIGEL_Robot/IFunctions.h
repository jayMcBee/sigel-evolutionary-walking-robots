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
#ifndef SIGEL_ROBOT_IFUNCTIONS
#define SIGEL_ROBOT_IFUNCTIONS

#include <pointvector.h>
#include <matrix.h>
#include <qstring.h>

namespace SIGEL_Robot {

        double tolerantACos( double cosInput );

        DL_matrix rotationMatrix(DL_vector v, DL_Scalar phi);

        void calculateAnyJoint (DL_vector VD, DL_vector VE, DL_vector VF,
                                DL_vector VA, DL_vector VB, DL_vector VC,
                                double winkel, double verschiebung,
                                DL_matrix & mo, DL_vector & vt,
                                QString someIdentifier);
}

#endif
