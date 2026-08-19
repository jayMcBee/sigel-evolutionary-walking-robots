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
// may be ignored so far
#include "SIGEL_Robot/SIG_Mirtich.h"
#include <newmatap.h>
#ifdef _WINDOWS
#include <float.h>
#endif

#define X 0
#define Y 1
#define Z 2

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

namespace SIGEL_Robot {

        void SIG_Mirtich::compProjectionIntegrals(FACE *f)
        {
                double a0, a1, da;
                double b0, b1, db;
                double a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
                double a1_2, a1_3, b1_2, b1_3;
                double C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
                double Cab, Kab, Caab, Kaab, Cabb, Kabb;
                int i;
                
                P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

                for (i = 0; i < f->numVerts; i++) {
                        a0 = f->poly->verts[f->verts[i]][cA];
                        b0 = f->poly->verts[f->verts[i]][cB];
                        a1 = f->poly->verts[f->verts[(i+1) % f->numVerts]][cA];
                        b1 = f->poly->verts[f->verts[(i+1) % f->numVerts]][cB];
                        da = a1 - a0;
                        db = b1 - b0;
                        a0_2 = a0 * a0; a0_3 = a0_2 * a0; a0_4 = a0_3 * a0;
                        b0_2 = b0 * b0; b0_3 = b0_2 * b0; b0_4 = b0_3 * b0;
                        a1_2 = a1 * a1; a1_3 = a1_2 * a1; 
                        b1_2 = b1 * b1; b1_3 = b1_2 * b1;
                        
                        C1 = a1 + a0;
                        Ca = a1*C1 + a0_2; Caa = a1*Ca + a0_3; Caaa = a1*Caa + a0_4;
                        Cb = b1*(b1 + b0) + b0_2; Cbb = b1*Cb + b0_3; Cbbb = b1*Cbb + b0_4;
                        Cab = 3*a1_2 + 2*a1*a0 + a0_2; Kab = a1_2 + 2*a1*a0 + 3*a0_2;
                        Caab = a0*Cab + 4*a1_3; Kaab = a1*Kab + 4*a0_3;
                        Cabb = 4*b1_3 + 3*b1_2*b0 + 2*b1*b0_2 + b0_3;
                        Kabb = b1_3 + 2*b1_2*b0 + 3*b1*b0_2 + 4*b0_3;
                        
                        P1 += db*C1;
                        Pa += db*Ca;
                        Paa += db*Caa;
                        Paaa += db*Caaa;
                        Pb += da*Cb;
                        Pbb += da*Cbb;
                        Pbbb += da*Cbbb;
                        Pab += db*(b1*Cab + b0*Kab);
                        Paab += db*(b1*Caab + b0*Kaab);
                        Pabb += da*(a1*Cabb + a0*Kabb);
                }
                
                P1 /= 2.0;
                Pa /= 6.0;
                Paa /= 12.0;
                Paaa /= 20.0;
                Pb /= -6.0;
                Pbb /= -12.0;
                Pbbb /= -20.0;
                Pab /= 24.0;
                Paab /= 60.0;
                Pabb /= -60.0;
        }

        void SIG_Mirtich::compFaceIntegrals(FACE *f)
        { 
                double *n, w;
                double k1, k2, k3, k4;
                
                compProjectionIntegrals(f);
                
                w = f->w;
                n = f->norm;
                k1 = 1 / n[cC]; k2 = k1 * k1; k3 = k2 * k1; k4 = k3 * k1;
                
                Fa = k1 * Pa;
                Fb = k1 * Pb;
                Fc = -k2 * (n[cA]*Pa + n[cB]*Pb + w*P1);
                
                Faa = k1 * Paa;
                Fbb = k1 * Pbb;
                Fcc = k3 * (SQR(n[cA])*Paa + 2*n[cA]*n[cB]*Pab + SQR(n[cB])*Pbb
                            + w*(2*(n[cA]*Pa + n[cB]*Pb) + w*P1));
                
                Faaa = k1 * Paaa;
                Fbbb = k1 * Pbbb;
                Fccc = -k4 * (CUBE(n[cA])*Paaa + 3*SQR(n[cA])*n[cB]*Paab
                              + 3*n[cA]*SQR(n[cB])*Pabb + CUBE(n[cB])*Pbbb
                              + 3*w*(SQR(n[cA])*Paa + 2*n[cA]*n[cB]*Pab + SQR(n[cB])*Pbb)
                              + w*w*(3*(n[cA]*Pa + n[cB]*Pb) + w*P1));
                
                Faab = k1 * Paab;
                Fbbc = -k2 * (n[cA]*Pabb + n[cB]*Pbbb + w*Pbb);
                Fcca = k3 * (SQR(n[cA])*Paaa + 2*n[cA]*n[cB]*Paab + SQR(n[cB])*Pabb
                             + w*(2*(n[cA]*Paa + n[cB]*Pab) + w*Pa));
        }

        void SIG_Mirtich::compVolumeIntegrals(POLYHEDRON *p)
        {
                FACE *f;
                double nx, ny, nz;
                int i;

                T0 = T1[X] = T1[Y] = T1[Z] 
                        = T2[X] = T2[Y] = T2[Z] 
                        = TP[X] = TP[Y] = TP[Z] = 0;
                
                for (i = 0; i < p->numFaces; i++) {

                        f = &p->faces[i];

                        nx = fabs(f->norm[X]);
                        ny = fabs(f->norm[Y]);
                        nz = fabs(f->norm[Z]);
                        if (nx > ny && nx > nz) cC = X;
                        else cC = (ny > nz) ? Y : Z;
                        cA = (cC + 1) % 3;
                        cB = (cA + 1) % 3;

                        compFaceIntegrals(f);

                        T0 += f->norm[X] * ((cA == X) ? Fa : ((cB == X) ? Fb : Fc));

                        T1[cA] += f->norm[cA] * Faa;
                        T1[cB] += f->norm[cB] * Fbb;
                        T1[cC] += f->norm[cC] * Fcc;
                        T2[cA] += f->norm[cA] * Faaa;
                        T2[cB] += f->norm[cB] * Fbbb;
                        T2[cC] += f->norm[cC] * Fccc;
                        TP[cA] += f->norm[cA] * Faab;
                        TP[cB] += f->norm[cB] * Fbbc;
                        TP[cC] += f->norm[cC] * Fcca;
                }

                T1[X] /= 2; T1[Y] /= 2; T1[Z] /= 2;
                T2[X] /= 3; T2[Y] /= 3; T2[Z] /= 3;
                TP[X] /= 2; TP[Y] /= 2; TP[Z] /= 2;
        }

        void SIG_Mirtich::compFaceNormal (FACE *f)
        {
                POLYHEDRON *p = f->poly;
                double dx1, dy1, dz1, dx2, dy2, dz2, nx, ny, nz, len;

                dx1 = p->verts[f->verts[1]][X] - p->verts[f->verts[0]][X];
                dy1 = p->verts[f->verts[1]][Y] - p->verts[f->verts[0]][Y];
                dz1 = p->verts[f->verts[1]][Z] - p->verts[f->verts[0]][Z];
                dx2 = p->verts[f->verts[2]][X] - p->verts[f->verts[1]][X];
                dy2 = p->verts[f->verts[2]][Y] - p->verts[f->verts[1]][Y];
                dz2 = p->verts[f->verts[2]][Z] - p->verts[f->verts[1]][Z];
                nx = dy1 * dz2 - dy2 * dz1;
                ny = dz1 * dx2 - dz2 * dx1;
                nz = dx1 * dy2 - dx2 * dy1;
                len = sqrt(nx * nx + ny * ny + nz * nz);
                f->norm[X] = nx / len;
                f->norm[Y] = ny / len;
                f->norm[Z] = nz / len;
                f->w = - f->norm[X] * p->verts[f->verts[0]][X]
                        - f->norm[Y] * p->verts[f->verts[0]][Y]
                        - f->norm[Z] * p->verts[f->verts[0]][Z];
        }

        void SIG_Mirtich::compute (void)
        {
                if (computed)
                        return;
                computed = true;
                
                POLYHEDRON p;
                p.verts = new tridouble [geom->getNumVertices ()];
                p.faces = new FACE [geom->getNumPolygons ()];

                p.numVerts = geom->getNumVertices ();
                p.numFaces = geom->getNumPolygons ();

                for (int i = 0; i < p.numVerts; i++) {
                        DL_vector v = geom->getVertex (i);
                        p.verts [i][X] = v.get (X);
                        p.verts [i][Y] = v.get (Y);
                        p.verts [i][Z] = v.get (Z);
                }

                SIG_GeometryIterator it (geom);
                for (int k = 0; (k < p.numFaces) && it; k++) {
                        p.faces [k].poly = &p;
                        
                        SIG_Polygon const & pl = it.iterate ();
                        int numv = pl.getNumVertices ();
                        p.faces [k].numVerts = numv;
                        p.faces [k].verts = new int [numv];

                        for (int j = 0; j < numv; j++)
                                p.faces [k].verts [j] = pl.getVertexIndex (j);

                        compFaceNormal (&(p.faces [k]));
                }

                compVolumeIntegrals (&p);

                // Great - painstakingly built up, one call, and now
                // I get to delete the whole damn thing again.

                for (int j = 0; j < p.numFaces; j++)
                        delete [] p.faces [j].verts;
                delete [] p.faces;
                delete [] p.verts;
        }

        SIG_Mirtich::SIG_Mirtich (SIG_Geometry *geometr, QString nameOfGeom)
                : computed (false),
                  myExcName (nameOfGeom),
                  geom (geometr)
        { }

        SIG_Mirtich::~SIG_Mirtich ()
        { }

        void SIG_Mirtich::computePhysics (double density,
                                          DL_Scalar & masse,
                                          DL_vector & centreOfMass,
                                          DL_matrix & inertiaTensor)
        {
                double mass;
                double r[3];            /* center of mass */
                double J[3][3];         /* inertia tensor */

                compute ();

                mass = density * T0;

                r[X] = T1[X] / T0;
                r[Y] = T1[Y] / T0;
                r[Z] = T1[Z] / T0;

                /* compute inertia tensor */
                J[X][X] = density * (T2[Y] + T2[Z]);
                J[Y][Y] = density * (T2[Z] + T2[X]);
                J[Z][Z] = density * (T2[X] + T2[Y]);
                J[X][Y] = J[Y][X] = - density * TP[X];
                J[Y][Z] = J[Z][Y] = - density * TP[Y];
                J[Z][X] = J[X][Z] = - density * TP[Z];

                /* translate inertia tensor to center of mass */
                /*
                  J[X][X] -= mass * (r[Y]*r[Y] + r[Z]*r[Z]);
                  J[Y][Y] -= mass * (r[Z]*r[Z] + r[X]*r[X]);
                  J[Z][Z] -= mass * (r[X]*r[X] + r[Y]*r[Y]);
                  J[X][Y] = J[Y][X] += mass * r[X] * r[Y]; 
                  J[Y][Z] = J[Z][Y] += mass * r[Y] * r[Z]; 
                  J[Z][X] = J[X][Z] += mass * r[Z] * r[X];
                */

                for (int i = 0; i < 3; i++) {
#ifdef _WINDOWS
					    		if (_isnan (r [i])){
#else
                        if (isnan (r [i])){
#endif
                                throw SIG_CannotMirtich (__FILE__, __LINE__,
                                                         myExcName); }
                        centreOfMass.set (i, r[i]);
                }

                for (int k = 0; k < 3; k++)
                        for (int j = 0; j < 3; j++) {
#ifdef _WINDOWS
                                if (_isnan (J[k][j])){
#else
                                if (isnan (J[k][j])){
#endif
                                        throw SIG_CannotMirtich (__FILE__,
                                                                 __LINE__,
                                                                 myExcName); }
                                inertiaTensor.set (k, j, J[k][j]);
                        }

#ifdef _WINDOWS
                if (_isnan (mass)){
#else
                if (isnan (mass)){
#endif
                        throw SIG_CannotMirtich (__FILE__, __LINE__,
                                                 myExcName); }
                masse = mass;
        }

        void SIG_Mirtich::computeMajorAxes (DL_vector &centreOfMass,
                                            DL_vector &v1,
                                            DL_vector &v2,
                                            DL_vector &v3)
        {
                double density = 1.0;
                
                double mass;
                double r[3];            /* center of mass */
                double J[3][3];         /* inertia tensor */

                NEWMAT::SymmetricMatrix S (3);
                NEWMAT::DiagonalMatrix D (3);
                NEWMAT::Matrix V (3, 3);
                NEWMAT::ColumnVector a1, a2, a3;

                compute ();

                mass = density * T0;

                r[X] = T1[X] / T0;
                r[Y] = T1[Y] / T0;
                r[Z] = T1[Z] / T0;

                /* compute inertia tensor */
                J[X][X] = density * (T2[Y] + T2[Z]);
                J[Y][Y] = density * (T2[Z] + T2[X]);
                J[Z][Z] = density * (T2[X] + T2[Y]);
                J[X][Y] = J[Y][X] = - density * TP[X];
                J[Y][Z] = J[Z][Y] = - density * TP[Y];
                J[Z][X] = J[X][Z] = - density * TP[Z];

                /* translate inertia tensor to center of mass */
                J[X][X] -= mass * (r[Y]*r[Y] + r[Z]*r[Z]);
                J[Y][Y] -= mass * (r[Z]*r[Z] + r[X]*r[X]);
                J[Z][Z] -= mass * (r[X]*r[X] + r[Y]*r[Y]);
                J[X][Y] = J[Y][X] += mass * r[X] * r[Y]; 
                J[Y][Z] = J[Z][Y] += mass * r[Y] * r[Z]; 
                J[Z][X] = J[X][Z] += mass * r[Z] * r[X];

                for (int i = 0; i < 3; i++)
                        centreOfMass.set (i, r[i]);

                for (int k = 0; k < 3; k++)
                        for (int j = 0; j <= k; j++)
                                S (k + 1, j + 1) = J [k] [j];

                NEWMAT::Jacobi (S, D, V);

                a1 = V.Column (1);
                a2 = V.Column (2);
                a3 = V.Column (3);

                for (int l = 0; l < 3; l++) {
                        v1.set (l, a1 (l+1));
                        v2.set (l, a2 (l+1));
                        v3.set (l, a3 (l+1));
                }
                
                v1.normalize ();
                v2.normalize ();
                v3.normalize ();
        }

        void SIG_Mirtich::moveToOriginAndMajorAxes (DL_vector & translation,
                                                    DL_matrix & rotation)
        {
                DL_matrix axes;

                computeMajorAxes (translation, axes.c0, axes.c1, axes.c2);

                axes.invert (&rotation);
                translation.timesis (-1.0);

                geom->translate (translation);
                geom->rotate (rotation);

                computed = false;
        }
        
} // namespace
