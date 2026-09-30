/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_TOOLS_SIG_VECTOR_H
#define SIGEL_TOOLS_SIG_VECTOR_H

#include <cmath>

/**
 * A 3-vector. Its members are named and computed as in the Dynamo
 * library's DL_vector; only those SIGEL uses are here.
 *
 * The default constructor leaves x, y and z uninitialised.
 */
class SIG_Vector
{
public:
  double x, y, z;

  SIG_Vector() {}
  SIG_Vector( SIG_Vector *v ) { assign( v ); }
  SIG_Vector( double nx, double ny, double nz ) : x( nx ), y( ny ), z( nz ) {}

  void init( double nx, double ny, double nz ) { x = nx; y = ny; z = nz; }
  void assign( SIG_Vector *v ) { x = v->x; y = v->y; z = v->z; }

  double norm() { return std::sqrt( x*x + y*y + z*z ); }

  // A zero vector stays zero.
  void normalize()
  {
    double l = norm();
    if (l != 0.0) {
      x /= l;
      y /= l;
      z /= l;
    }
  }

  double inprod( SIG_Vector *v ) { return x*v->x + y*v->y + z*v->z; }

  void plusis( SIG_Vector *v ) { x = x+v->x; y = y+v->y; z = z+v->z; }
  void minusis( SIG_Vector *v ) { x = x-v->x; y = y-v->y; z = z-v->z; }
  void timesis( double f ) { x = x*f; y = y*f; z = z*f; }

  bool equal( SIG_Vector *v ) { return x == v->x && y == v->y && z == v->z; }

  // An index outside 0..2 reads 0 and writes nothing.
  double get( int r )
  {
    switch (r) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    }
    return 0;
  }

  void set( int r, double f )
  {
    switch (r) {
    case 0: x = f; break;
    case 1: y = f; break;
    case 2: z = f; break;
    }
  }

  // nv = this x v
  void crossprod( SIG_Vector *v, SIG_Vector *nv )
  {
    nv->x = y*v->z - z*v->y;
    nv->y = z*v->x - x*v->z;
    nv->z = x*v->y - y*v->x;
  }
};

#endif // SIGEL_TOOLS_SIG_VECTOR_H
