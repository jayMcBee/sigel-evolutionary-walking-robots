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
#ifndef SIGEL_ROBOT_SIG_MIRTICH_H
#define SIGEL_ROBOT_SIG_MIRTICH_H

namespace SIGEL_Robot { class SIG_Mirtich; }

#include <math.h>
#include "SIGEL_Tools/SIG_Vector.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_Polygon.h"

#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include <qstring.h>
#include <vector>

namespace SIGEL_Robot {

        class SIG_Mirtich {
                struct POLYHEDRON;
                typedef double tridouble [3];
                struct FACE {
                        int numVerts;
                        double norm[3];
                        double w;
                        std::vector< int > verts;
                        POLYHEDRON *poly;
                };
                struct POLYHEDRON {
                        int numVerts, numFaces;
                        tridouble *verts;
                        FACE *faces;
                };
        protected:
                // this is to what we refer
                SIG_Geometry const *geom;
                // alpha, beta, gamma
                int cA, cB, cC;
                // projection integrals (return values)
                double P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;
                // face integrals (ditto)
                double Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;
                // volume integrals (ditto, i guess)
                double T0, T1[3], T2[3], TP[3];
                // just a flag
                bool computed;
                QString geometryName;

                void compProjectionIntegrals (FACE *f);
                void compFaceIntegrals (FACE *f);
                void compVolumeIntegrals (POLYHEDRON *p);
                void compFaceNormal (FACE *f);
                void compute ();

        public:
                SIG_Mirtich (SIG_Geometry const *geom, QString nameOfGeom);
                virtual ~SIG_Mirtich() = default;

                void computePhysics (double density,
                                     double & mass,
                                     SIG_Vector & centreOfMass,
                                     SIG_Matrix & inertiaTensor);
                void invalidate () { computed = false; }
 		double getT0()  { return T0; }
        };

}

#endif
