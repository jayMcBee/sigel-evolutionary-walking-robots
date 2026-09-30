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
#ifndef SIGEL_TOOLS_DL_MATRIX_H
#define SIGEL_TOOLS_DL_MATRIX_H

#include "SIGEL_Tools/DL_vector.h"

/**
 * A 3x3 matrix, stored as three column vectors. Its members are named and
 * computed as in the Dynamo library's DL_matrix; only those SIGEL uses are
 * here.
 *
 * The default constructor leaves the elements uninitialised.
 */
class DL_matrix
{
public:
  DL_vector c0;
  DL_vector c1;
  DL_vector c2;

  DL_matrix() {}

  void makeone()
  {
    c0.x = c1.y = c2.z = 1.0;
    c0.y = c0.z = c1.x = c1.z = c2.x = c2.y = 0.0;
  }

  void assign( DL_matrix *m )
  {
    c0.assign( &m->c0 );
    c1.assign( &m->c1 );
    c2.assign( &m->c2 );
  }

  // r is the row, c the column. An index outside 0..2 reads 0 and writes nothing.
  double get( int r, int c )
  {
    switch (c) {
    case 0: return c0.get( r );
    case 1: return c1.get( r );
    case 2: return c2.get( r );
    }
    return 0;
  }

  void set( int r, int c, double f )
  {
    switch (c) {
    case 0: c0.set( r, f ); break;
    case 1: c1.set( r, f ); break;
    case 2: c2.set( r, f ); break;
    }
  }

  // nm = this * m
  void times( DL_matrix *m, DL_matrix *nm )
  {
    nm->c0.x = c0.x * m->c0.x + c1.x * m->c0.y + c2.x * m->c0.z;
    nm->c1.x = c0.x * m->c1.x + c1.x * m->c1.y + c2.x * m->c1.z;
    nm->c2.x = c0.x * m->c2.x + c1.x * m->c2.y + c2.x * m->c2.z;

    nm->c0.y = c0.y * m->c0.x + c1.y * m->c0.y + c2.y * m->c0.z;
    nm->c1.y = c0.y * m->c1.x + c1.y * m->c1.y + c2.y * m->c1.z;
    nm->c2.y = c0.y * m->c2.x + c1.y * m->c2.y + c2.y * m->c2.z;

    nm->c0.z = c0.z * m->c0.x + c1.z * m->c0.y + c2.z * m->c0.z;
    nm->c1.z = c0.z * m->c1.x + c1.z * m->c1.y + c2.z * m->c1.z;
    nm->c2.z = c0.z * m->c2.x + c1.z * m->c2.y + c2.z * m->c2.z;
  }

  // nv = this * v
  void times( DL_vector *v, DL_vector *nv )
  {
    nv->x = c0.x * v->x + c1.x * v->y + c2.x * v->z;
    nv->y = c0.y * v->x + c1.y * v->y + c2.y * v->z;
    nv->z = c0.z * v->x + c1.z * v->y + c2.z * v->z;
  }

  // nv = transpose(this) * v
  void transposetimes( DL_vector *v, DL_vector *nv )
  {
    nv->x = c0.x * v->x + c0.y * v->y + c0.z * v->z;
    nv->y = c1.x * v->x + c1.y * v->y + c1.z * v->z;
    nv->z = c2.x * v->x + c2.y * v->y + c2.z * v->z;
  }
};

#endif // SIGEL_TOOLS_DL_MATRIX_H
